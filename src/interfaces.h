! interfaces to use instead of EXTERNAL for type checking

      INTERFACE

         SUBROUTINE FUNI(AP,NDIM,U,UOLD,ICP,PAR,IJAC,F,DFDU,DFDP)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
         INTEGER, INTENT(IN) :: ICP(*),NDIM,IJAC
         DOUBLE PRECISION, INTENT(IN) :: UOLD(NDIM)
         DOUBLE PRECISION, INTENT(INOUT) :: U(NDIM),PAR(*)
         DOUBLE PRECISION, INTENT(OUT) :: F(NDIM)
         DOUBLE PRECISION, INTENT(INOUT) :: DFDU(NDIM,NDIM),DFDP(NDIM,*)
         END SUBROUTINE FUNI

         SUBROUTINE BCNI(AP,NDIM,PAR,ICP,NBC,U0,U1,F,IJAC,DBC)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
         INTEGER, INTENT(IN) :: NDIM,ICP(*),NBC,IJAC
         DOUBLE PRECISION, INTENT(INOUT) :: U0(NDIM),U1(NDIM),PAR(*)
         DOUBLE PRECISION, INTENT(OUT) :: F(NBC)
         DOUBLE PRECISION, INTENT(INOUT) :: DBC(NBC,*)
         END SUBROUTINE BCNI

         SUBROUTINE ICNI(AP,NDIM,PAR,ICP,NINT,U,UOLD,UDOT,UPOLD,F,IJ,D)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
         INTEGER, INTENT(IN) :: ICP(*),NDIM,NINT,IJ
         DOUBLE PRECISION, INTENT(IN) :: UOLD(NDIM),UDOT(NDIM)
         DOUBLE PRECISION, INTENT(IN) :: UPOLD(NDIM)
         DOUBLE PRECISION, INTENT(INOUT) :: U(NDIM),PAR(*)
         DOUBLE PRECISION, INTENT(OUT) :: F(NINT)
         DOUBLE PRECISION, INTENT(INOUT) :: D(NINT,*)
         END SUBROUTINE ICNI

         SUBROUTINE STPNBVI(AP,PAR,ICP,NTSR,NCOLRS,RLD,UPS,UDOPS,TM,ND)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
         INTEGER, INTENT(IN) :: ICP(*)
         INTEGER, INTENT(INOUT) :: NTSR,NCOLRS
         INTEGER, INTENT(OUT) :: ND
         DOUBLE PRECISION, INTENT(OUT) :: PAR(*),RLD(*)
         DOUBLE PRECISION, INTENT(OUT) :: UPS(AP%NDIM,0:*)
         DOUBLE PRECISION, INTENT(OUT) :: UDOPS(AP%NDIM,0:*),TM(0:*)
         END SUBROUTINE STPNBVI

         SUBROUTINE STPNAEI(AP,PAR,ICP,U,UDOT,NODIR)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(INOUT) :: AP
         INTEGER, INTENT(IN) :: ICP(*)
         INTEGER, INTENT(OUT) :: NODIR
         DOUBLE PRECISION, INTENT(OUT) :: PAR(*),U(*),UDOT(*)
         END SUBROUTINE STPNAEI

         SUBROUTINE PVLI(AP,ICP,UPS,NDIM,PAR)
         USE AUTO_CONSTANTS, ONLY: AUTOPARAMETERS
         TYPE(AUTOPARAMETERS), INTENT(IN) :: AP
         INTEGER, INTENT(IN) :: ICP(*),NDIM
         DOUBLE PRECISION, INTENT(IN) :: UPS(NDIM,0:*)
         DOUBLE PRECISION, INTENT(INOUT) :: PAR(*)
         END SUBROUTINE PVLI

      END INTERFACE
