!
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!                    Algebraic Problems
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!
MODULE AE

  USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS

  IMPLICIT NONE
  PRIVATE
  PUBLIC :: AUTOAE,STPNUS,STPNAE

CONTAINS

! ---------- ------
  SUBROUTINE AUTOAE(AP,PAR,ICP,ICU,FUNI,STPNAEI,THL,THU,IUZ,VUZ)

! This is the entry subroutine for algebraic systems.

    USE AUTOMPI

    TYPE(AUTOPARAMETERS)AP
    INTEGER ICP(*),ICU(*),IUZ(*)
    DOUBLE PRECISION PAR(*),THL(*),THU(*),VUZ(*)

    include 'interfaces.h'

    IF(MPIIAM()>0)THEN
       IF(MPIWFI(.FALSE.))THEN
          RETURN
       ENDIF
    ENDIF
    THU(AP%NDIM+1)=THL(1)
    CALL CNRLAE(AP,PAR,ICP,ICU,FUNI,STPNAEI,THU,IUZ,VUZ)

  END SUBROUTINE AUTOAE

! ---------- ------
  SUBROUTINE CNRLAE(AP,PAR,ICP,ICU,FUNI,STPNAEI,THU,IUZ,VUZ)

    USE IO
    USE MESH
    USE SUPPORT

! Controls the bifurcation analysis of algebraic problems

    include 'interfaces.h'

    TYPE(AUTOPARAMETERS)AP
    INTEGER ICP(*),ICU(*),IUZ(*)
    DOUBLE PRECISION PAR(*),VUZ(*),THU(*)
! Local
    DOUBLE PRECISION, ALLOCATABLE :: &
         AA(:,:),U(:),UDOT(:),STUD(:,:),STU(:,:),UZR(:)
    LOGICAL IPOS,CHNG,FOUND
    INTEGER NDIM,IPS,IRS,ILP,IADS,ISP,ISW,NUZR,MXBF,NBIFS,NBFCS,ITPST,IBR
    INTEGER ITNW,ITP,I,IUZR,LAB,NINS,NBIF,NBFC,NODIR,NIT,NTOT,NTOP,ITDS
    DOUBLE PRECISION DS,DSMAX,RDS,DSOLD,EPSS
    LOGICAL ISTOP,CHECKEDHB
    INTEGER STOPCNTS(-9:9)

    NDIM=AP%NDIM
    IPS=AP%IPS
    IRS=AP%IRS
    ILP=AP%ILP
    IADS=AP%IADS
    ISP=AP%ISP
    ISW=AP%ISW
    NUZR=AP%NUZR
    MXBF=AP%MXBF
    NBIFS=ABS(MXBF)
    IF(MXBF>0)THEN
       NBFCS=2*NBIFS
    ELSE
       NBFCS=NBIFS
    ENDIF
    ITPST=AP%ITPST
    IBR=AP%IBR

    DS=AP%DS

    ALLOCATE(AA(NDIM+1,NDIM+1),U(NDIM+1),UDOT(NDIM+1))
    ALLOCATE(STUD(NBIFS,NDIM+1),STU(NBIFS,NDIM+1),UZR(NUZR+4),EVV(NDIM))

    NINS=0
    AP%NINS=NINS
    RDS=DS
    NBIF=0
    IPOS=.TRUE.
    LAB=0
    AP%LAB=LAB

    DO I=1,NDIM
       U(I)=0.d0
       UDOT(I)=0.d0
    ENDDO

! Generate the starting point

    NODIR=1
    CALL STPNAEI(AP,PAR,ICP,U,UDOT,NODIR)
    CALL PVLSAE(AP,U,PAR)

! Determine a suitable starting label and branch number

    CALL NEWLAB(AP)

! Write constants

    CALL STHD(AP,ICP)

    DO NBFC=0,NBFCS !bifurcation switch loop

       DO I=1,NUZR+4
          UZR(I)=0.d0
       ENDDO

       NTOT=0
       AP%NTOT=NTOT
       CALL INITSTOPCNTS(STOPCNTS)
       ISTOP=.FALSE.
       NIT=1

       IF(IRS.EQ.0) THEN
          ITP=9+10*ITPST
       ELSE
          ITP=0
       ENDIF
       AP%ITP=ITP
       U(NDIM+1)=PAR(ICP(1))

! Starting procedure  (to get direction vector) :

       IF(NODIR==1.AND.ISW>=0)THEN
          CALL STPRAE(AP,PAR,ICP,FUNI,U,UDOT,THU,0,AA)
       ELSEIF(IRS/=0.AND.ISW<0)THEN
          CALL STPRAE(AP,PAR,ICP,FUNI,U,UDOT,THU,1,AA)
       ELSEIF(ABS(IPS)==1.OR.IPS==11)THEN
          CALL STPRAE(AP,PAR,ICP,FUNI,U,UDOT,THU,-1,AA)
       ENDIF
       IF(ABS(IPS)==1.OR.IPS==11)THEN
          ! Get stability
          UZR(NUZR+4)=FNHBAE(AP,PAR,CHNG,AA)
       ENDIF

! Store plotting data for first point on the bifurcating branch
! or for the starting point

       CALL STPLAE(AP,PAR,ICP,ICU,U,UDOT,NIT,ISTOP)

       IF(.NOT.ISTOP)THEN

! Provide initial approximation to the second point on the branch and
! determine the second point on the bifurcating or original branch
          CALL STEPAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT,ISW<0)

          IF(ISW<0.OR.NIT==0)THEN
             IF(ABS(IPS)==1.OR.IPS==11)THEN
                ! Get stability
                UZR(NUZR+4)=FNHBAE(AP,PAR,CHNG,AA)
             ENDIF
             ! Store plotting data for second point :
             CALL STPLAE(AP,PAR,ICP,ICU,U,UDOT,NIT,ISTOP)
          ENDIF
       ENDIF

       UZR(NUZR+4)=0.d0
       DO WHILE(.NOT.ISTOP) ! branch computation loop
          ITP=0
          AP%ITP=ITP
          NINS=AP%NINS

! Find the next solution point on the branch
          CALL STEPAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT)
          ISTOP=NIT==0
          DSOLD=RDS

          CHECKEDHB=.FALSE.
          DO IUZR=1,NUZR+4
             IF(ISTOP)EXIT
             IF(IUZR<=NUZR)THEN
                ITP=-4 ! Check for user supplied parameter output parameter-values.
             ELSEIF(IUZR==NUZR+1)THEN
                ITP=2  ! Check for fold
             ELSEIF(IUZR==NUZR+2)THEN
                ITP=1  ! Check for branch point
             ELSEIF(IUZR==NUZR+3)THEN
                ITP=-3  ! Check for Bogdanov-Takens bifurcation
             ELSEIF(IUZR==NUZR+4)THEN
                ITP=3  ! Check for Hopf bifurcation
             ENDIF
             IF(CHECKSP(ITP,IPS,ILP,ISP))THEN
                ! Check for UZ, LP, BP, HB
                IF(ITP==3)THEN
                   CHECKEDHB=.TRUE.
                ENDIF
                CALL LCSPAE(AP,RDS,PAR,ICP,IUZR,FUNI,AA,&
                     U,UDOT,UZR(IUZR),THU,IUZ,VUZ,NIT,ISTOP,FOUND)
                IF(FOUND)THEN
                   IF(ITP<0)THEN
                      ITP=ITP-10*ITPST
                   ELSEIF(ITP==3.AND.IPS==-1)THEN
                      ITDS=AP%ITDS
                      EPSS=AP%EPSS
                      ITP=TPSPAE(EPSS,ITPST,PAR(11))
                      PAR(11)=PAR(11)*ITDS
                   ELSE
                      ITP=ITP+10*ITPST
                   ENDIF
                   AP%ITP=ITP
                   IF(IUZ(IUZR)>=0.AND..NOT.STOPPED(ITP,STOPCNTS))THEN
                      IF(MOD(ITP,10)==-4)THEN
                         UZR(1:NUZR)=0.d0
                      ELSE
                         UZR(NUZR+1:NUZR+4)=0.d0
                      ENDIF
                      IF(MOD(ITP,10)==1)THEN
                         ! Check for branch point, and if so store data :
                         CALL STBIF(NDIM,NBIF,NBIFS,STUD,STU,U,UDOT)
                      ENDIF
                   ELSE
                      ! *Stop at the first found bifurcation
                      ISTOP=.TRUE.
                   ENDIF
                ENDIF
             ENDIF
          ENDDO
          IF(.NOT.CHECKEDHB.AND.(ABS(IPS)==1.OR.IPS==11))THEN
             ! Still determine eigenvalue information and stability
             ! for situations where ISTOP=-1 or SP switched off HB detection
             UZR(NUZR+4)=FNHBAE(AP,PAR,CHNG,AA)
          ENDIF
          ITP=AP%ITP 
          IF(ITP/=0.AND.MOD(ITP,10)/=-4)THEN
             ! for plotter: use stability of previous point
             ! for bifurcation points
             AP%NINS=NINS
          ENDIF

! Store plotting data on unit 7 :

          NTOT=AP%NTOT
          CALL STPLAE(AP,PAR,ICP,ICU,U,UDOT,NIT,ISTOP)

! Adapt the stepsize along the branch

          ITP=AP%ITP
          RDS=DSOLD
          IF(IADS.NE.0 .AND. MOD(NTOT,IADS).EQ.0 &
               .AND. ( MOD(ITP,10).EQ.0 .OR. MOD(ITP,10).EQ.4) )THEN
             ITNW=AP%ITNW
             NTOP=MOD(NTOT-1,9999)+1
             DSMAX=AP%DSMAX
             CALL ADPTDS(NIT,ITNW,IBR,NTOP,DSMAX,RDS)
             AP%RDS=RDS
          ENDIF
       ENDDO !from branch computation loop

       IF(NBIF==0.OR.NBFC>=NBFCS)EXIT

       ! Initialize computation of the next bifurcating branch.

       CALL SWPNT(AP,DS,PAR,ICP,RDS,NBIF,NBIFS,STUD,STU,U,UDOT,IPOS)

       IF(IPOS)THEN
          NBIF=NBIF-1
       ENDIF

       IF(.NOT.IPOS .OR. MXBF.LT.0 )IBR=IBR+1
       AP%IBR=IBR

       ! IRS and ISW are for internal use: don't store in AP!
       IRS=1
       ISW=-1
    ENDDO !from bifurcation switch loop

    DEALLOCATE(EVV,AA,U,UDOT,STUD,STU,UZR)
  END SUBROUTINE CNRLAE

! ---------- ------
  SUBROUTINE STPNUS(AP,PAR,ICP,U,UDOT,NODIR)

! Gets the starting data from user supplied STPNT

    USE AUTO_CONSTANTS, ONLY : UVALS, PARVALS, unames, parnames
    USE IO, ONLY: NAMEIDX
    TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
    INTEGER, INTENT(IN) :: ICP(*)
    INTEGER, INTENT(OUT) :: NODIR
    DOUBLE PRECISION, INTENT(OUT) :: U(*),UDOT(*)
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)

    INTEGER NDIM,I
    DOUBLE PRECISION T

    NDIM=AP%NDIM
    T=0.d0
    U(:NDIM)=0.d0

    CALL STPNT(NDIM,U,PAR,T)

! override parameter/point values with values from constants file

    DO I=1,SIZE(UVALS)
       U(NAMEIDX(UVALS(I)%INDEX,unames))=UVALS(I)%VAR
    ENDDO
    DO I=1,SIZE(PARVALS)
       PAR(NAMEIDX(PARVALS(I)%INDEX,parnames))=PARVALS(I)%VAR
    ENDDO

    UDOT(1)=0
    NODIR=1
    
  END SUBROUTINE STPNUS

! ---------- ------
  SUBROUTINE STPNAE(AP,PAR,ICP,U,UDOT,NODIR)

    USE IO

! Gets the starting data from unit 3
    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    INTEGER, INTENT(IN) :: ICP(*)
    INTEGER, INTENT(OUT) :: NODIR
    DOUBLE PRECISION, INTENT(OUT) :: U(*),UDOT(*)
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)

    INTEGER NFPR,NFPRS,I,IPS,ITP,ISW,ITDS
    INTEGER,ALLOCATABLE :: ICPRS(:)

    NFPRS=GETNFPR3()
    ALLOCATE(ICPRS(NFPRS))
    ICPRS(:)=0
    CALL READLB(AP,ICPRS,U,UDOT,PAR)
  
! Take care of the case where the free parameters have been changed at
! the restart point.

    NODIR=0
    NFPR=AP%NFPR
    IF(NFPRS/=NFPR)THEN
       NODIR=1
    ELSE
       DO I=1,NFPR
          IF(ICPRS(I)/=ICP(I)) THEN
             NODIR=1
             EXIT
          ENDIF
       ENDDO
    ENDIF
    DEALLOCATE(ICPRS)

    IPS=AP%IPS
    ISW=AP%ISW
    ITP=AP%ITP
    IF(IPS==-1.AND.ISW==-1.AND.ITP==7)THEN
       ! period doubling for maps: set iteration count
       ITDS=AINT(PAR(11))
       AP%ITDS=ITDS
    ENDIF

  END SUBROUTINE STPNAE

! ---------- ------
  SUBROUTINE STPRAE(AP,PAR,ICP,FUNI,U,UDOT,THU,IPERP,AA)

    USE SUPPORT

! Finds the second point on the initial solution branch.

    include 'interfaces.h'

    TYPE(AUTOPARAMETERS) AP
    INTEGER ICP(*),IPERP
    DOUBLE PRECISION U(*),UDOT(AP%NDIM+1),THU(*),PAR(*)
    DOUBLE PRECISION AA(AP%NDIM+1,AP%NDIM+1)

! Local
    DOUBLE PRECISION, ALLOCATABLE :: AAA(:,:),F(:),DFDU(:,:),DFDP(:,:)
    INTEGER NDIM,IID,NPAR,I
    DOUBLE PRECISION SIGN,SS

    NDIM=AP%NDIM
    IID=AP%IID
    NPAR=AP%NPAR

    ALLOCATE(F(NDIM),DFDU(NDIM,NDIM),DFDP(NDIM,NPAR))
    DFDU(:,:)=0.d0
    DFDP(:,:)=0.d0

! Determine the direction of the branch at the starting point

    CALL FUNI(AP,NDIM,U,U,ICP,PAR,2,F,DFDU,DFDP)

    IF(IPERP==1)THEN
       AA(:NDIM,:NDIM)=DFDU(:,:)
       AA(:NDIM,NDIM+1)=DFDP(:,ICP(1))
       AA(NDIM+1,:)=UDOT(:)
       IF(IID.GE.3)CALL WRJAC(NDIM+1,NDIM+1,AA,F)
       CALL NLVC(NDIM+1,NDIM+1,1,AA,UDOT)
    ELSEIF(IPERP==0)THEN
       ALLOCATE(AAA(NDIM,NDIM+1))
       AAA(:,1:NDIM)=DFDU(:,:)
       AAA(:,NDIM+1)=DFDP(:,ICP(1))
       IF(IID.GE.3)CALL WRJAC(NDIM,NDIM+1,AAA,F)
       CALL NLVC(NDIM,NDIM+1,1,AAA,UDOT)
       DEALLOCATE(AAA)
    ENDIF

! Scale and make sure that the PAR(ICP(1))-dot is positive.

    SS=0.d0
    DO I=1,NDIM+1
       SS=SS+THU(I)*UDOT(I)**2
    ENDDO

    SIGN=1.d0
    IF(UDOT(NDIM+1)<0.d0.AND.IPERP/=1)SIGN=-1.d0
    UDOT(:)=SIGN/SQRT(SS)*UDOT(:)

! Get the Jacobian for stability computation.
    AA(:NDIM,:NDIM)=DFDU(:,:)
    AA(:NDIM,NDIM+1)=DFDP(:,ICP(1))
    AA(NDIM+1,:)=UDOT(:)

    DEALLOCATE(F,DFDU,DFDP)

  END SUBROUTINE STPRAE

! ---------- ------
  SUBROUTINE STEPAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT,SW)

! This is the subroutine for computing solution branches. It solves
! the equations for finding the next point on the branch at distance RDS
! from the current point. RDS is adapted if there is no convergence.

! It also controls the computation of the second point on a bifurcating branch.
! This point is required to lie in a hyper-plane at distance DS from the
! branch point. This hyper-plane is parallel to the tangent of the
! known branch at the branch point.

    USE MESH
    include 'interfaces.h'

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    INTEGER, INTENT(IN) :: ICP(*)
    INTEGER, INTENT(OUT) :: NIT
    DOUBLE PRECISION, INTENT(IN) :: THU(*)
    DOUBLE PRECISION, INTENT(OUT) :: AA(AP%NDIM+1,AP%NDIM+1)
    DOUBLE PRECISION, INTENT(INOUT) :: U(*),UDOT(*),PAR(*),RDS
    LOGICAL, OPTIONAL, INTENT(IN) :: SW
! Local
    INTEGER IADS,IID,ITNW,IBR,NTOT,NTOP
    LOGICAL BSW
    DOUBLE PRECISION DSMIN,DSMAX
    CHARACTER (LEN=7) FIXEDMINIMUM

    IADS=AP%IADS
    IID=AP%IID
    ITNW=AP%ITNW
    IBR=AP%IBR
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1

    DSMIN=AP%DSMIN
    DSMAX=AP%DSMAX

    BSW=.FALSE.
    IF(PRESENT(SW))BSW=SW
    
    DO
       CALL SOLVAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT,BSW)
       IF(NIT>0)THEN
          CALL PVLSAE(AP,U,PAR)
          RETURN
       ENDIF

! Maximum number of iterations has been reached.

       IF(IADS.EQ.0)EXIT

! Reduce stepsize and try again.

       CALL ADPTDS(ITNW,ITNW,IBR,NTOP,DSMAX,RDS)
       AP%RDS=RDS
       IF(ABS(RDS).LT.DSMIN)EXIT
       IF(IID.GE.2)THEN
          WRITE(9,"(I4,I6,A)")IBR,NTOP,' NOTE:Retrying step'
       ENDIF
    ENDDO

! Minimum stepsize reached.

    IF(IADS==0)THEN
       FIXEDMINIMUM='fixed'
    ELSE
       FIXEDMINIMUM='minimum'
    ENDIF
    IF(BSW)THEN
       WRITE(9,"(I4,I6,A,A,A)")&
         IBR,NTOP,' NOTE:No convergence when switching branches with ',&
         FIXEDMINIMUM,' step size'
    ELSE
       WRITE(9,"(I4,I6,A,A,A)")&
            IBR,NTOP,' NOTE:No convergence with ',FIXEDMINIMUM,' step size'
    ENDIF
  END SUBROUTINE STEPAE

! ---------- ------
  SUBROUTINE SOLVAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT,BSW)

! This subroutine contains the main predictor-corrector loop
! Input: U, UDOT, PAR : extended solution
!        RDS: step-size
!        AP AP ICP, THU, FUNI: constants & function interface
!        BSW: if true, switch branches, else do normal continuation
! Output: NIT: number of iterations taken to converge
!         after successful convergence (NIT>0):
!           AP%DET contains the determinant 
!           AA contains the extended Jacobian matrix
!           U, UDOT, PAR are updated
!         else (NIT==0):
!           U, UDOT, PAR are not updated

    USE IO
    USE SUPPORT

    include 'interfaces.h'

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    INTEGER, INTENT(IN) :: ICP(*)
    INTEGER, INTENT(OUT) :: NIT
    LOGICAL, INTENT(IN) :: BSW
    DOUBLE PRECISION, INTENT(IN) :: RDS,THU(*)
    DOUBLE PRECISION, INTENT(OUT) :: AA(AP%NDIM+1,AP%NDIM+1)
    DOUBLE PRECISION, INTENT(INOUT) :: U(AP%NDIM+1),UDOT(AP%NDIM+1),PAR(*)

! Local
    INTEGER NDIM,IID,ITNW,IBR,NTOT,NTOP,NIT1,NPAR,I,K,NDM
    LOGICAL CONVERGED
    DOUBLE PRECISION, ALLOCATABLE :: RHS(:),DU(:), &
         DFDU(:,:),DFDP(:,:),UOLD(:)
    DOUBLE PRECISION EPSL,EPSU,SS,UMX,DUMX,RDRLM,RDUMX,DELREF,ADU,AU,DET
    CHARACTER (LEN=*), PARAMETER :: O9 = & 
     "(' Branch ',I2,' N=',I5,1X,'IT=',I2,1X,'PAR(',I2,')=', &
        &ES11.3,1X,'U=',7ES11.3)"

    NDIM=AP%NDIM
    IID=AP%IID
    ITNW=AP%ITNW
    NDM=AP%NDM
    IBR=AP%IBR
    NPAR=AP%NPAR
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1

    EPSL=AP%EPSL
    EPSU=AP%EPSU

    ALLOCATE(RHS(NDIM+1),DU(NDIM+1),DFDU(NDIM,NDIM),DFDP(NDIM,NPAR))
    DFDU(:,:)=0.d0
    DFDP(:,:)=0.d0

    ALLOCATE(UOLD(NDIM+1))
    UOLD(:)=U(:)
    U(:)=UOLD(:)+RDS*UDOT(:)

! Write additional output on unit 9 if requested :

    IF(IID.GE.2)THEN
       CALL WRBAR("=",47)
       IF(BSW)THEN
          WRITE(9,O9)IBR,NTOP,0,ICP(1), &
               U(NDIM+1),(U(I),I=1,MIN(NDIM,6))
       ELSE
          WRITE(9,100)
          WRITE(9,101)IBR,NTOP+1,0,U(NDIM+1),RNRMV(NDM,U)
100       FORMAT(/,'  BR    PT  IT         PAR',11X,'L2-NORM')
101       FORMAT(I4,I6,I4,5X,2ES14.5)
       ENDIF
    ENDIF

! Call user-supplied FUNC to evaluate the right hand side of the
! differential equation and its derivatives :

    CONVERGED=.FALSE.
    DELREF=0
    DO NIT1=1,ITNW

       NIT=NIT1
       PAR(ICP(1))=U(NDIM+1)
       CALL FUNI(AP,NDIM,U,UOLD,ICP,PAR,2,RHS,DFDU,DFDP)

! Set up the Jacobian matrix and the right hand side :

       DO I=1,NDIM
          AA(I,NDIM+1)=DFDP(I,ICP(1))
          RHS(I)=-RHS(I)
          DO K=1,NDIM
             AA(I,K)=DFDU(I,K)
          ENDDO
       ENDDO
       SS=0.d0
       IF(BSW)THEN
          ! Branch switch
          DO K=1,NDIM+1
             AA(NDIM+1,K)=THU(K)*UDOT(K)
             SS=SS+THU(K)*(U(K)-UOLD(K)-RDS*UDOT(K))*UDOT(K)
          ENDDO
          RHS(NDIM+1)=-SS
       ELSE
          DO K=1,NDIM+1
             AA(NDIM+1,K)=2.d0*THU(K)*(U(K)-UOLD(K))/RDS
             SS=SS+THU(K)*(U(K)-UOLD(K))**2
          ENDDO
          RHS(NDIM+1)=RDS-SS/RDS
       ENDIF

! Use Gauss elimination with pivoting to solve the linearized system :

       IF(IID.GE.5)CALL WRJAC(NDIM+1,NDIM+1,AA,RHS)
       CALL GEL(NDIM+1,AA,1,DU,RHS,DET)
       AP%DET=DET

! Add the Newton increments :

       DO I=1,NDIM+1
          U(I)=U(I)+DU(I)
       ENDDO
       DUMX=0.d0
       UMX=0.d0
       DO I=1,NDIM
          ADU=ABS(DU(I))
          AU=ABS(U(I))
          IF(AU>UMX)UMX=AU
          IF(ADU>DUMX)DUMX=ADU
       ENDDO

       IF(IID.GE.2)THEN
          IF(BSW)THEN
             WRITE(9,O9)IBR,NTOP,NIT,ICP(1),U(NDIM+1),(U(I),I=1,MIN(NDIM,6))
          ELSE
             WRITE(9,101)IBR,NTOP+1,NIT,U(NDIM+1),RNRMV(NDM,U)
          ENDIF
       ENDIF

! Check whether relative error has reached user-supplied tolerance :

       RDRLM=ABS(DU(NDIM+1))/(1.d0+ABS(U(NDIM+1)))
       RDUMX=DUMX/(1.d0+UMX)
       IF(RDRLM.LE.EPSL.AND.RDUMX.LE.EPSU)THEN
! Recompute Jacobian for test functions
          PAR(ICP(1))=U(NDIM+1)
          CALL FUNI(AP,NDIM,U,UOLD,ICP,PAR,2,RHS,DFDU,DFDP)
          DO I=1,NDIM
             AA(I,NDIM+1)=DFDP(I,ICP(1))
             DO K=1,NDIM
                AA(I,K)=DFDU(I,K)
             ENDDO
          ENDDO
          DO K=1,NDIM+1
             AA(NDIM+1,K)=UDOT(K)
          ENDDO
          IF(IID.GE.2)WRITE(9,*)
! This subroutine determines an initial approximation to the next
! solution on a branch by extrapolating from the two preceding points.
          UDOT(:)=(U(:)-UOLD(:))/RDS
          CONVERGED=.TRUE.
          EXIT
       ENDIF

       IF(.NOT.BSW)THEN
          IF(NIT.EQ.1)THEN
             DELREF=20*MAX(RDRLM,RDUMX)
          ELSE
             IF(MAX(RDRLM,RDUMX).GT.DELREF)EXIT
          ENDIF
       ENDIF

    ENDDO
    IF(.NOT.CONVERGED)THEN
       DO I=1,NDIM+1
          U(I)=UOLD(I)
       ENDDO
       PAR(ICP(1))=U(NDIM+1)
       NIT=0
    ENDIF
    DEALLOCATE(RHS,DU,DFDU,DFDP,UOLD)

  END SUBROUTINE SOLVAE
!
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!               Detection of Singular Points
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!
! ---------- ------
  SUBROUTINE LCSPAE(AP,RDS,PAR,ICP,IUZR,FUNI,AA, &
       U,UDOT,Q,THU,IUZ,VUZ,NIT,ISTOP,FOUND)

    USE SUPPORT

    DOUBLE PRECISION, PARAMETER :: HMACH=1.0d-7

! This subroutine uses the secant method to accurately locate special
! points (branch points, folds, Hopf bifurcations, user zeroes).
! These are characterized as zeroes of the function FNCS.
! This subroutine calls CONT and STEPAE with varying stepsize RDS.
! The special point is assumed to have been found with sufficient
! accuracy if the ratio between RDS and the user supplied value of
! DS is less than the user-supplied toler EPSS.

    include 'interfaces.h'

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    INTEGER ICP(*),IUZ(*),NIT,IUZR
    DOUBLE PRECISION RDS,PAR(*),THU(*),VUZ(*)
    DOUBLE PRECISION AA(AP%NDIM+1,*),U(*),UDOT(*),Q
    LOGICAL, INTENT(OUT) :: ISTOP, FOUND

    LOGICAL CHNG
    INTEGER IID,ITMX,IBR,ITLCSP,NTOT
    DOUBLE PRECISION DS,DSMAX,EPSS,Q0,Q1,DQ,S,S0,S1,RRDS

    ISTOP=.FALSE.
    FOUND=.FALSE.
    IID=AP%IID
    ITMX=AP%ITMX
    IBR=AP%IBR

    DS=AP%DS
    DSMAX=AP%DSMAX
    EPSS=AP%EPSS

! Check whether FNCS has changed sign.

    Q0=Q
    Q1=FNCS(AP,PAR,CHNG,AA,U,IUZ,VUZ,IUZR)
    NTOT=AP%NTOT
    ! do not test via Q0*Q1 to avoid overflow.
    IF((Q0>=0.AND.Q1>=0) .OR. (Q0<=0.AND.Q1<=0) .OR. (.NOT. CHNG))THEN
       Q=Q1
       RETURN
    ENDIF

! Use the secant method for the first step:

    S0=0.d0
    S1=RDS
    DQ=Q0-Q1
    RDS=Q1/DQ*(S1-S0)
    DO ITLCSP=0,ITMX
       RDS=(1.d0+HMACH)*RDS
       S=S1+RDS

! Return if relative tolerance has been met :

       RRDS=ABS(RDS)/(1+SQRT(ABS(DS*DSMAX)))
       IF(RRDS.LT.EPSS)THEN
          FOUND=.TRUE.
          Q=0.d0
          WRITE(9,102)RDS
          RETURN
       ENDIF

! If requested write additional output on unit 9 :

       IF(IID.GE.2)THEN
          WRITE(9,101)ITLCSP,RDS
       ENDIF

       CALL STEPAE(AP,PAR,ICP,FUNI,RDS,AA,U,UDOT,THU,NIT)
       IF(NIT==0)THEN
          ISTOP=.TRUE.
          Q=0.d0
          RETURN
       ENDIF

       Q=FNCS(AP,PAR,CHNG,AA,U,IUZ,VUZ,IUZR)
!        Use Mueller's method with bracketing for subsequent steps
       CALL MUELLER(Q0,Q1,Q,S0,S1,S,RDS)
    ENDDO
    WRITE(9,103)IBR,MOD(NTOT-1,9999)+1
    Q=0.d0

101 FORMAT(' ==> Location of special point :  Iteration ',I3, &
         '  Step size = ',ES13.5)
102 FORMAT(' ==> Location of special point : ', &
         ' Convergence.   Step size = ',ES13.5)
103 FORMAT(I4,I6,' NOTE:Possible special point')
  END SUBROUTINE LCSPAE

! ------ --------- -------- ----
  DOUBLE PRECISION FUNCTION FNCS(AP,PAR,CHNG,AA,U,IUZ,VUZ,IUZR)

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
    LOGICAL, INTENT(OUT) :: CHNG
    DOUBLE PRECISION, INTENT(INOUT) :: AA(AP%NDIM+1,AP%NDIM+1)
    DOUBLE PRECISION, INTENT(IN) :: U(AP%NDIM)
    INTEGER, INTENT(IN) :: IUZ(*),IUZR
    DOUBLE PRECISION, INTENT(IN) :: VUZ(*)

    INTEGER NUZR

    NUZR=AP%NUZR

    IF(IUZR==NUZR+1)THEN
       FNCS=FNLPAE(AP,CHNG,AA)
    ELSEIF(IUZR==NUZR+2)THEN
       FNCS=FNBPAE(AP,CHNG)
    ELSEIF(IUZR==NUZR+3)THEN
       FNCS=FNBTAE(AP,CHNG,AA,U)
    ELSEIF(IUZR==NUZR+4)THEN
       FNCS=FNHBAE(AP,PAR,CHNG,AA)
    ELSE
       FNCS=FNUZAE(AP,PAR,CHNG,IUZ,VUZ,IUZR)
    ENDIF

  END FUNCTION FNCS

! ------ --------- -------- ------
  DOUBLE PRECISION FUNCTION FNBPAE(AP,CHNG)

    TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
    LOGICAL, INTENT(OUT) :: CHNG

    INTEGER IID,IBR,NTOT,NTOP
    DOUBLE PRECISION DET

    IID=AP%IID
    IBR=AP%IBR
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1

    DET=AP%DET
    FNBPAE=DET
    CHNG=.TRUE.

! If requested write additional output on unit 9 :

    IF(IID.GE.2)WRITE(9,101)IBR,NTOP+1,FNBPAE
101 FORMAT(I4,I6,9X,'BP   Function:',ES14.5)

  END FUNCTION FNBPAE

! ------ --------- -------- ------
  DOUBLE PRECISION FUNCTION FNLPAE(AP,CHNG,AA)

    USE SUPPORT

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    LOGICAL, INTENT(OUT) :: CHNG
    DOUBLE PRECISION, INTENT(INOUT) :: AA(AP%NDIM+1,AP%NDIM+1)
! Local
    DOUBLE PRECISION, ALLOCATABLE :: UD(:),AAA(:,:),RHS(:)

    INTEGER NDIM,IID,IBR,NTOT,NTOP
    DOUBLE PRECISION DET

    NDIM=AP%NDIM
    IID=AP%IID
    IBR=AP%IBR
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1

    ALLOCATE(AAA(NDIM+1,NDIM+1),RHS(NDIM+1))
    AAA(:,:)=AA(:,:)
    RHS(1:NDIM)=0.d0
    RHS(NDIM+1)=1.d0

    ALLOCATE(UD(NDIM+1))
    CALL GEL(NDIM+1,AAA,1,UD,RHS,DET)
!   don't store DET here: it is for a different matrix than
!   used with pseudo arclength continuation and sometimes has
!   a  different sign
    CALL NRMLZ(NDIM+1,UD)
    FNLPAE=UD(NDIM+1)
    DEALLOCATE(UD,AAA,RHS)
    AP%FLDF=FNLPAE
    CHNG=.TRUE.

! If requested write additional output on unit 9 :

    IF(IID.GE.2)WRITE(9,101)ABS(IBR),NTOP+1,FNLPAE
101 FORMAT(I4,I6,9X,'Fold Function:',ES14.5)

  END FUNCTION FNLPAE

! ------ --------- -------- ------
  DOUBLE PRECISION FUNCTION FNHBAE(AP,PAR,CHNG,AA)

    USE SUPPORT

    DOUBLE PRECISION, PARAMETER :: HMACH=1.0d-7,RLARGE=1.0d+30

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
    LOGICAL, INTENT(OUT) :: CHNG
    DOUBLE PRECISION, INTENT(INOUT) :: AA(AP%NDIM+1,AP%NDIM+1)
! Local
    COMPLEX(KIND(1.0D0)) ZTMP
    COMPLEX(KIND(1.0D0)), ALLOCATABLE :: EV(:)
    INTEGER NDIM,NDM,IPS,ISP,ISW,IID,IBR,NTOT,NTOP,NINS,NINS1,I,j,LOC,ITPST
    DOUBLE PRECISION a,AR,AREV,RIMHB,tol,trace,REV

    NDIM=AP%NDIM
    NDM=AP%NDM
    IPS=AP%IPS
    ISP=AP%ISP
    ISW=AP%ISW
    IID=AP%IID
    IBR=AP%IBR
    ITPST=AP%ITPST
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1
    ALLOCATE(EV(NDIM))

! INITIALIZE

    CHNG=.FALSE.

! Set tolerance for deciding if an eigenvalue is in the positive
! half-plane. Use, for example, tol=1d-3 for conservative systems.

! Try to guess whether the system is probably conservative or definitely not:
! the dimension is even and the trace 0 if it is conservative.
! In that case we use a tolerance to avoid detecting spurious
! Hopf bifurcations.

    tol=0.d0
    IF(MOD(NDM,2)==0)THEN
       trace=0.d0
       DO I=1,NDM
          trace=trace+AA(i,i)
       ENDDO
       a=0.d0
       DO i=1,NDM
          DO j=1,NDM
             IF(ABS(AA(i,j))>a)THEN
                a=ABS(AA(i,j))
             ENDIF
          ENDDO
       ENDDO
       IF(ABS(trace)<HMACH*a)THEN
          tol=1.d-5
       ENDIF
    ENDIF

! Compute the eigenvalues of the Jacobian

    CALL EIG(AP,NDM,NDIM+1,AA,EV)
    IF(IPS.EQ.-1)THEN
       DO I=1,NDM
          IF(REAL(EV(I)).NE.-1.d0 .OR. &
               AIMAG(EV(I)).NE. 0.d0)THEN
             EV(I)=LOG(1.d0+EV(I))
          ELSE
             EV(I)= CMPLX(-RLARGE,0.d0,KIND(1.0D0))
          ENDIF
       ENDDO
    ENDIF

! Order the eigenvalues by real part.

    DO I=1,NDM-1
       LOC=I
       DO J=I+1,NDM
          IF(REAL(EV(J)).GE.REAL(EV(LOC)))THEN
             LOC=J
          ENDIF
       ENDDO
       IF(LOC>I) THEN
          ZTMP=EV(LOC)
          EV(LOC)=EV(I)
          EV(I)=ZTMP
       ENDIF
    ENDDO

! Compute the smallest real part.

    AREV=HUGE(AREV)
    REV=0.d0
    LOC=0
    DO I=1,NDM
       IF(AIMAG(EV(I)).NE.0.d0.OR.(ISW==2.AND.ITPST==2.AND.IPS/=-1))THEN
          AR=ABS(REAL(EV(I)))
          IF(AR.LE.AREV)THEN
             AREV=AR
             LOC=I
          ENDIF
       ENDIF
    ENDDO
    IF(ISW==2.AND.ITPST==2.AND.IPS/=-1)THEN
       ! for Zero-Hopf compute one-but-smallest real part
       AREV=HUGE(AREV)
       DO I=1,NDM
          AR=ABS(REAL(EV(I)))
          IF(AR.LE.AREV.AND.I/=LOC)THEN
             AREV=AR
             REV=REAL(EV(I))
          ENDIF
       ENDDO
    ELSEIF(LOC>0)THEN
       REV=REAL(EV(LOC))
       RIMHB=ABS(AIMAG(EV(LOC)))
       IF(RIMHB.NE.0.d0.AND.ABS(ISW).LE.1)PAR(11)=PI(2.d0)/RIMHB
    ENDIF

! Count the number of eigenvalues with negative real part.

    NINS1=0
    DO I=1,NDM
       IF(REAL(EV(I)).LE.tol)NINS1=NINS1+1
    ENDDO

    IF((ISW==2.AND.(ITPST/=2.OR.IPS==-1)) .OR. ISP==0 .OR. ISP==3)THEN
       FNHBAE=0.d0
    ELSE
       FNHBAE=REV
    ENDIF
    AP%HBFF=FNHBAE
    NINS=AP%NINS
    IF(NINS1.NE.NINS)CHNG=.TRUE.
    NINS=NINS1
    AP%NINS=NINS

    IF(IID.GE.2)WRITE(9,101)ABS(IBR),NTOP+1,FNHBAE

    WRITE(9,102)ABS(IBR),NTOP+1,NINS
    IF(IPS.EQ.-1)THEN
       EVV(:)=EXP(EV(:))
    ELSE
       EVV(:)=EV(:)
    ENDIF
    DO I=1,NDM
       WRITE(9,103)ABS(IBR),NTOP+1,I,EVV(I)
    ENDDO

101 FORMAT(I4,I6,9X,'Hopf Function:',ES14.5)
102 FORMAT(/,I4,I6,9X,'Eigenvalues  :   Stable:',I4)
103 FORMAT(I4,I6,9X,'Eigenvalue',I3,":",2ES14.5)

    DEALLOCATE(EV)
  END FUNCTION FNHBAE

! ------ --------- -------- ------
  DOUBLE PRECISION FUNCTION FNBTAE(AP,CHNG,AA,U)

    USE SUPPORT, ONLY: NLVC, NRMLZ

    TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
    LOGICAL, INTENT(OUT) :: CHNG
    DOUBLE PRECISION, INTENT(IN) :: AA(AP%NDIM+1,AP%NDIM+1)
    DOUBLE PRECISION, INTENT(IN) :: U(AP%NDIM)
! Local
    DOUBLE PRECISION, ALLOCATABLE :: DFU(:,:),V(:)
    INTEGER NDM,NTOP,I

    FNBTAE = 0
    IF(AP%ISW/=2.OR.AP%ITPST/=2.OR.AP%IPS==-1.OR.AP%ISP==0.OR.AP%ISP==3)THEN
       RETURN
    ENDIF

    NDM=AP%NDM
    ALLOCATE(DFU(NDM,NDM),V(NDM))
    ! get null vector for the transposed Jacobian
    DO I=1,NDM
       DFU(1:NDM,I)=AA(I,1:NDM)
    ENDDO
    CALL NLVC(NDM,NDM,1,DFU,V)
    CALL NRMLZ(NDM,V)

    ! take the inner product with the null vector for the Jacobian
    FNBTAE = DOT_PRODUCT(U(NDM+1:2*NDM),V(1:NDM))

    DEALLOCATE(DFU,V)

    NTOP=MOD(AP%NTOT-1,9999)+1
    IF(AP%IID.GE.2)WRITE(9,101)ABS(AP%IBR),NTOP+1,FNBTAE
101 FORMAT(I4,I6,9X,'BT   Function:',ES14.5)

  END FUNCTION FNBTAE

! ------ --------- -------- ------
  DOUBLE PRECISION FUNCTION FNUZAE(AP,PAR,CHNG,IUZ,VUZ,IUZR)

    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
    LOGICAL, INTENT(OUT) :: CHNG
    INTEGER, INTENT(IN) :: IUZ(*),IUZR
    DOUBLE PRECISION, INTENT(IN) :: VUZ(*)

    INTEGER IID,IBR,NTOT,NTOP

    IID=AP%IID
    IBR=AP%IBR
    NTOT=AP%NTOT
    NTOP=MOD(NTOT-1,9999)+1

    FNUZAE=PAR(ABS(IUZ(IUZR)))-VUZ(IUZR)
    CHNG=.TRUE.

    IF(IID.GE.3)WRITE(9,101)ABS(IBR),NTOP+1,IUZR,FNUZAE
101 FORMAT(I4,I6,9X,'User Func.',I3,1X,ES14.5)

  END FUNCTION FNUZAE

! ------- -------- ------
  INTEGER FUNCTION TPSPAE(EPSS,ITPST,PERIOD)

! Determines type of secondary bifurcation of maps.
    
    USE SUPPORT, ONLY: PI

    INTEGER, INTENT(IN) :: ITPST
    DOUBLE PRECISION, INTENT(IN) :: EPSS, PERIOD

    IF(PERIOD-2 <= PERIOD/PI(1d0)*SQRT(EPSS))THEN
!       ** period doubling
       TPSPAE=7+10*ITPST
    ELSEIF(PERIOD /= 0 .AND. PERIOD < PI(2d0)/SQRT(EPSS))THEN
!       ** torus (Neimark-Sacker) bifurcation
       TPSPAE=8+10*ITPST
    ELSE
!       ** something else... (very large PERIOD: close to fold)
       TPSPAE=0
    ENDIF

  END FUNCTION TPSPAE
!
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!                   Branch Switching for Algebraic Problems
!-----------------------------------------------------------------------
!-----------------------------------------------------------------------
!
! ---------- -----
  SUBROUTINE STBIF(NDIM,NBIF,NBIFS,STUD,STU,U,UDOT)

    USE SUPPORT

! Stores branching data in the following arrays :
!        STU    ( the solution vector U | PAR(ICP(1)) )
!        STUD   ( U-dot | PAR(ICP(1))-dot )
! Here the vector ( U-dot, PAR(ICP(1))-dot ) is the direction vector of
! known branch at this point.

    INTEGER, INTENT(IN) :: NDIM,NBIFS
    INTEGER, INTENT(INOUT) :: NBIF
    DOUBLE PRECISION, INTENT(OUT) :: STUD(NBIFS,NDIM+1),STU(NBIFS,NDIM+1)
    DOUBLE PRECISION, INTENT(IN) :: U(NDIM+1),UDOT(NDIM+1)

! Keep track of the number of branch points stored.

    IF(NBIF==NBIFS)RETURN
    NBIF=NBIF+1

    STU(NBIF,:)=U(:)
    STUD(NBIF,:)=UDOT(:)

  END SUBROUTINE STBIF

! ---------- -----
  SUBROUTINE SWPNT(AP,DS,PAR,ICP,RDS,NBIF,NBIFS,STUD,STU,U,UDOT,IPOS)

! This subroutine retrieves the branching data U, U-dot, PAR(ICP(1)),
! PAR(ICP(1))-dot. If this initialization corresponds to the computation
! of the bifurcating branch in opposite direction, then only the sign of
!  the stepsize ( DS ) along the branch is reversed.

    TYPE(AUTOPARAMETERS) AP
    INTEGER ICP(*)
    INTEGER, INTENT(IN) :: NBIF,NBIFS
    DOUBLE PRECISION, INTENT(IN) :: DS
    DOUBLE PRECISION, INTENT(OUT) :: RDS
    DOUBLE PRECISION PAR(*),U(*),UDOT(*),STUD(NBIFS,*),STU(NBIFS,*)
    LOGICAL IPOS

    INTEGER NDIM,ISW,MXBF,I,I1

    NDIM=AP%NDIM
    ISW=AP%ISW
    MXBF=AP%MXBF

    RDS=DS
    IF(.NOT.IPOS)RDS=-DS
    DO I=1,NDIM+1
       U(I)=STU(1,I)
       UDOT(I)=STUD(1,I)
    ENDDO
    PAR(ICP(1))=U(NDIM+1)
    IF(ABS(ISW).EQ.2)PAR(ICP(2))=U(NDIM)

    IF(MXBF.GE.0)THEN
       IPOS=.NOT.IPOS
    ENDIF
    IF(.NOT.IPOS)RETURN

    DO I=1,NBIF-1
       DO I1=1,NDIM+1
          STU(I,I1)=STU(I+1,I1)
          STUD(I,I1)=STUD(I+1,I1)
       ENDDO
    ENDDO

  END SUBROUTINE SWPNT

! ---------- ------
  SUBROUTINE STPLAE(AP,PAR,ICP,ICU,U,UDOT,NIT,ISTOP)

    USE IO
    USE SUPPORT

! Stores the bifurcation diagram on unit 7 (Algebraic Problems).
! Every line written contains, in order, the following:
!
!  IBR    : The label of the branch.
!  NTOT   : The index of the point on the branch.
!           (Points are numbered consecutively along a branch).
!           If IPS=1, 11, or -1, then the sign of NTOT indicates stability :
!            - = stable , + = unstable, unknown, or not relevant.
!  ITP    : An integer indicating the type of point :
!
!             1  (BP)  :   Branch point.
!             2  (LP)  :   Fold.
!             3  (HB)  :   Hopf bifurcation point.
!             4  (  )  :   Output point (Every NPR steps along branch).
!            -4  (UZ)  :   Output point (Zero of user function).
!             9  (EP)  :   End point of branch, normal termination.
!            -9  (MX)  :   End point of branch, abnormal termination.
!
!  LAB        : The label of a special point.
!  PAR(ICP(1)): The principal parameter.
!  A          : The L2-norm of the solution vector, or other measure of
!               the solution (see the user-supplied parameter IPLT).
!  U          : The first few components of the solution vector.
!  PAR(ICP(*)): Further free parameters (if any).
!
    TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
    INTEGER, INTENT(IN) :: ICP(*),ICU(*),NIT
    DOUBLE PRECISION, INTENT(IN) :: U(*),UDOT(*)
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
    LOGICAL, INTENT(INOUT) :: ISTOP

    INTEGER NDIM,IPS,ISW,IPLT,NMX,NPR,NDM,ITP,ITPST,IBR
    INTEGER NTOT,NTOTS,IAB,LAB,LABW,NINS
    DOUBLE PRECISION RL0,RL1,A0,A1,AMP

    NDIM=AP%NDIM
    IPS=AP%IPS
    ISW=AP%ISW
    IPLT=AP%IPLT
    NMX=AP%NMX
    NPR=AP%NPR
    NDM=AP%NDM
    ITP=AP%ITP
    ITPST=AP%ITPST
    IBR=AP%IBR

    RL0=AP%RL0
    RL1=AP%RL1
    A0=AP%A0
    A1=AP%A1

    NTOT=AP%NTOT
    NTOT=NTOT+1
    AP%NTOT=NTOT

    CALL PVLSAE(AP,U,PAR)

! ITP is set to 4 every NPR steps along a branch, and the entire
! solution is written on unit 8.

    IF(NPR.NE.0)THEN
       IF(MOD(NTOT,NPR).EQ.0 .AND. MOD(ITP,10).EQ.0)ITP=4+10*ITPST
       AP%ITP=ITP
    ENDIF

! CHECK WHETHER LIMITS OF THE BIFURCATION DIAGRAM HAVE BEEN REACHED :

    IAB=ABS(IPLT)

    IF(IAB.LE.NDIM .AND. IAB.GT.0)THEN
       AMP=U(IAB)
    ELSE IF(IPLT.GT.NDIM.AND.IPLT.LE.2*NDIM)THEN
       AMP=U(IPLT-NDIM)
    ELSE IF(IPLT.GT.2*NDIM.AND.IPLT.LE.3*NDIM)THEN
       AMP=U(IPLT-2*NDIM)
    ELSE
       AMP=RNRMV(NDM,U)
    ENDIF

    IF(NIT==0)THEN
!        Maximum number of iterations reached somewhere.
       ISTOP=.TRUE.
       ITP=-9-10*ITPST
       AP%ITP=ITP
    ELSEIF(U(NDIM+1).LT.RL0.OR.U(NDIM+1).GT.RL1 &
         .OR. AMP.LT.A0.OR.AMP.GT.A1 &
         .OR. NTOT.EQ.NMX) THEN
       ISTOP=.TRUE.
       ITP=9+10*ITPST
       AP%ITP=ITP
    ENDIF

    LABW=0
    IF(MOD(ITP,10).NE.0)THEN
       LAB=AP%LAB
       LAB=LAB+1
       AP%LAB=LAB
       LABW=LAB
    ENDIF

! Determine stability and print output on units 6 and 7.

    NTOTS=NTOT
    NINS=AP%NINS
    IF((ABS(IPS)==1.OR.IPS==11) .AND. ABS(ISW)<=1)THEN
       IF(NINS.EQ.NDIM)NTOTS=-NTOT
    ENDIF
    CALL WRLINE(AP,PAR,ICU,IBR,NTOTS,LABW,AMP,U)

! Write restart information for multi-parameter analysis :

    IF(LABW.NE.0)CALL WRTSP8(AP,PAR,ICP,LABW,U,UDOT)
!
  END SUBROUTINE STPLAE

! ---------- ------
  SUBROUTINE WRTSP8(AP,PAR,ICP,LAB,U,UDOT)

    USE COMPAT
    USE AUTO_CONSTANTS, ONLY: IPS, NDIMU => NDIM

! Write restart information on singular points, plotting points, etc.,
! on unit 8.

    TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
    INTEGER, INTENT(IN) :: ICP(*),LAB
    DOUBLE PRECISION, INTENT(IN) :: U(*),UDOT(*)
    DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)

    INTEGER NDIM,ISW,ITP,IBR,NFPR,NPAR,NTOT,NROWPR,MTOT,NAR,NTPL,I,K
    INTEGER NPARI
    DOUBLE PRECISION T

    NDIM=AP%NDIM
    ISW=AP%ISW
    NPARI=AP%NPARI
    ITP=AP%ITP
    IBR=AP%IBR
    NFPR=AP%NFPR
    NPAR=AP%NPAR
    NTOT=AP%NTOT

    NTPL=1
    NAR=NDIM+1

    NROWPR=(NDIM+7)/7+(NDIM+6)/7 + (NFPR+6)/7 + (NPAR+6)/7 + (NFPR+19)/20
    PAR(ICP(1))=U(NDIM+1)
    T=0.d0

    MTOT=MOD(NTOT-1,9999)+1
    WRITE(8,101)IBR,MTOT,ITP,LAB,NFPR,ISW,NTPL,NAR,NROWPR,1,0,NPAR,&
         NPARI,NDIMU,IPS,0
    WRITE(8,102)T,(U(I),I=1,NDIM)
! Write the free parameter indices:
    WRITE(8,103)(ICP(I),I=1,NFPR)
! Write the direction of the branch:
    WRITE(8,102)UDOT(NDIM+1),(UDOT(NDIM-NFPR+I),I=2,NFPR)
    WRITE(8,102)(UDOT(K),K=1,NDIM)
! Write the parameter values.
    WRITE(8,102)(PAR(I),I=1,NPAR)

101 FORMAT(6I6,I8,I6,I8,7I5)
102 FORMAT(4X,7ES19.10)
103 FORMAT(20I5)

    CALL AUTOFLUSH(8)
  END SUBROUTINE WRTSP8

! ---------- ------
  SUBROUTINE WRJAC(M,N,AA,RHS)

    INTEGER, INTENT(IN) :: M,N
    DOUBLE PRECISION, INTENT(IN) :: AA(M,N),RHS(M)
    INTEGER I,J

    WRITE(9,"(A)")' Residual vector :'
    WRITE(9,"(1X,12E10.3)")RHS(:),(0d0,I=M+1,N)
    WRITE(9,"(A)")' Jacobian matrix :'
    DO I=1,M
       WRITE(9,"(1X,12E10.3)")(AA(I,J),J=1,N)
    ENDDO
    DO I=M+1,N
       WRITE(9,"(1X,12E10.3)")(0d0,J=1,N)
    ENDDO

  END SUBROUTINE WRJAC

! ---------- ------
  SUBROUTINE PVLSAE(AP,U,PAR)

    TYPE(AUTOPARAMETERS)AP
    DOUBLE PRECISION U(*),PAR(*)

    INTEGER NDM

    CALL SETPAE(AP)
    NDM=AP%NDM
    CALL PVLS(NDM,U,PAR)

  END SUBROUTINE PVLSAE

! ---------- ------
  SUBROUTINE SETPAE(AP)

    USE SUPPORT
    TYPE(AUTOPARAMETERS), TARGET :: AP

    AV=>AP

  END SUBROUTINE SETPAE

END MODULE AE
