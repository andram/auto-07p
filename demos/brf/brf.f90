!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 
!     brf  :  A parabolic PDE (the Brusselator)
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 
!     (Discretized in space by fourth order finite differences)
!---------------------------------------------------------------------- 
!----------------------------------------------------------------------
! NOTE: The values of the constants NE and NX are defined below as a
!       module.
!
!      NE  :  the dimension of the PDE system
!      NX  :  the number of space intervals for the discretization 
!
! The AUTO-constant NDIM must be set equal to the value of NE*(NX-1)
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 

      MODULE brf
        SAVE
        INTEGER, PARAMETER :: NE=2, NX=6
        INTEGER, PARAMETER :: NN=NX-1
        DOUBLE PRECISION D0(NN,NN),D2(NN,NN),DI(NN,NN),DD(NN,NN),RI(NN,NN)
      END MODULE brf

      SUBROUTINE FF(NE,U,PAR,F) 
!     ---------- -- 
!     Define the nonlinear term

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NE
      DOUBLE PRECISION, INTENT(IN) :: U(NE),PAR(*)
      DOUBLE PRECISION, INTENT(OUT) :: F(NE)

      DOUBLE PRECISION X,Y,A,B

        X=U(1)
        Y=U(2)
        A=PAR(1)
        B=PAR(2)

        F(1)= X**2*Y - (B+1)*X + A
        F(2)=-X**2*Y + B*X

      END SUBROUTINE FF

      SUBROUTINE SETDC(NE,DC,PAR) 
!     ---------- ----- 
!     Set the diffusion constants (constant, or in terms of PAR)

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NE
      DOUBLE PRECISION, INTENT(OUT) :: DC(NE)
      DOUBLE PRECISION, INTENT(IN) :: PAR(*)

        DC(1)=PAR(3)/PAR(5)**2
        DC(2)=PAR(4)/PAR(5)**2

      END SUBROUTINE SETDC

      SUBROUTINE SETBC(NE,PAR,U0,U1) 
!     ---------- ----- 
! Set the boundary values (to be kept fixed in time)

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NE
      DOUBLE PRECISION, INTENT(IN) :: PAR(*)
      DOUBLE PRECISION, INTENT(OUT) :: U0(NE),U1(NE)

      DOUBLE PRECISION A,B

        A=PAR(1)
        B=PAR(2)

        U0(1)=A
        U0(2)=B/A
        U1(1)=A
        U1(2)=B/A


      END SUBROUTINE SETBC

      SUBROUTINE STPNT(NDIM,U,PAR,T)
!     ---------- ----- 
! Define the starting stationary solution on the spatial mesh

      USE brf
      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM
      DOUBLE PRECISION, INTENT(INOUT) :: U(NN,NE),PAR(*)
      DOUBLE PRECISION, INTENT(IN) :: T

      DOUBLE PRECISION A,B,Dx,Dy,RL
      INTEGER I

! Set the parameter values
        A=2.d0
        B=5.45d0
        Dx=0.008d0
        Dy=0.004d0
        RL=0.4

        PAR(1)=A
        PAR(2)=B
        PAR(3)=Dx
        PAR(4)=Dy
        PAR(5)=RL

! Set the starting solution at space-points i/NX, i=1,2,...,NX-1
        DO I=1,NX-1
          U(I,1)=A
          U(I,2)=B/A
        ENDDO

      END SUBROUTINE STPNT
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 
!                Problem-independent subroutines
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 

      SUBROUTINE FUNC(NDIM,U,ICP,PAR,IJAC,F,DFDU,DFDP) 
!     ---------- ---- 

      USE brf
      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM, ICP(*), IJAC
      DOUBLE PRECISION, INTENT(IN) :: U(NN,NE), PAR(*)
      DOUBLE PRECISION, INTENT(OUT) :: F(NN,NE)
      DOUBLE PRECISION, INTENT(INOUT) :: DFDU(NDIM,NDIM), DFDP(NDIM,*)

      DOUBLE PRECISION W(NE),FW(NE),DC(NE)
      DOUBLE PRECISION U0(NE),U1(NE),F0(NE),F1(NE),V(NN,NE)

      INTEGER I,J,K

! Problem-independent initialization :

        CALL SETDC(NE,DC,PAR)
        CALL SETBC(NE,PAR,U0,U1)
        CALL FF(NE,U0,PAR,F0)
        CALL FF(NE,U1,PAR,F1)

        DO I=1,NN
          DO J=1,NE
            V(I,J)= DI(I, 1)*( DC(J)*NX**2*U0(J) + F0(J)/12 ) &
                  + DI(I,NN)*( DC(J)*NX**2*U1(J) + F1(J)/12 )
          ENDDO
        ENDDO

        DO I=1,NN
          DO K=1,NE
            W(K)=U(I,K)
          ENDDO
          CALL FF(NE,W,PAR,FW)
          DO J=1,NE
            F(I,J)=V(I,J) + FW(J)
            DO K=1,NN
              F(I,J)=F(I,J)+DC(J)*DD(I,K)*U(K,J)
            ENDDO
          ENDDO
        ENDDO

      END SUBROUTINE FUNC

      SUBROUTINE GENCF(PAR)
!     ---------- -----

      USE brf
      IMPLICIT NONE
      DOUBLE PRECISION, INTENT(IN) :: PAR(*)

      INTEGER IR(NN),IC(NN),I,J,K
      DOUBLE PRECISION S,DET

      DO I=1,NN
        DO J=1,NN
          D0(I,J)=0
          D2(I,J)=0
          DI(I,J)=0
          DD(I,J)=0
          RI(I,J)=0
        ENDDO
        D0(I,I)=10.d0/12.d0
        D2(I,I)=-2*NX**2
        RI(I,I)=1
      ENDDO

      DO I=1,NN-1
        D0(I+1,I)=1.d0/12.d0
        D0(I,I+1)=1.d0/12.d0
        D2(I+1,I)=NX**2
        D2(I,I+1)=NX**2   
      ENDDO

      CALL GE(0,NN,NN,D0,NN,NN,DI,NN,RI,IR,IC,DET)

      DO I=1,NN
        DO J=1,NN
          S=0.d0
          DO K=1,NN
            S=S+DI(I,K)*D2(K,J)
          ENDDO
          DD(I,J)=S
        ENDDO
      ENDDO

      END SUBROUTINE GENCF

      SUBROUTINE BCND 
      END SUBROUTINE BCND

      SUBROUTINE ICND 
      END SUBROUTINE ICND

      SUBROUTINE FOPT 
      END SUBROUTINE FOPT
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 

      SUBROUTINE PVLS(NDIM,U,PAR)
!     ---------- ----

      USE brf
      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM
      DOUBLE PRECISION, INTENT(IN) :: U(NDIM)
      DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
      LOGICAL, SAVE :: ifrst = .TRUE.

! Problem-independent initialization :
      IF(ifrst)THEN
         CALL GENCF(PAR)
         ifrst=.FALSE.
      ENDIF

      END SUBROUTINE PVLS
