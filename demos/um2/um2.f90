!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 
!   AUTO demo um2
!---------------------------------------------------------------------- 
!---------------------------------------------------------------------- 

      SUBROUTINE FUNC(NDIM,U,ICP,PAR,IJAC,F,DFDU,DFDP) 
!     ---------- ---- 

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM,ICP(*),IJAC
      DOUBLE PRECISION, INTENT(IN) :: U(NDIM),PAR(*)
      DOUBLE PRECISION, INTENT(OUT) :: F(NDIM)
      DOUBLE PRECISION, INTENT(INOUT) :: DFDU(NDIM,*),DFDP(NDIM,*)

      DOUBLE PRECISION eps, Period, x, y

	eps    = PAR(2)
 	Period = PAR(11)

	x = U(1)
	y = U(2)

	F(1) =  Period * ( eps * x - y**3 )
	F(2) =  Period * (       y + x**3 )
 
      END SUBROUTINE FUNC
!---------------------------------------------------------------------- 

      SUBROUTINE STPNT(NDIM,U,PAR,T) 
!     ---------- ----- 

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM
      DOUBLE PRECISION, INTENT(INOUT) :: U(NDIM),PAR(*)
      DOUBLE PRECISION, INTENT(IN) :: T

      DOUBLE PRECISION, PARAMETER :: &
           theta = 0.25, &
           eps   = 0.5, &
           r0    = 0.1, &
           r1    = r0, &
           Period= 1.d-4
      DOUBLE PRECISION PI

       PI = 4*ATAN(1.D0)

       PAR(1:4) = (/theta,eps,r0,r1/)
       PAR(11)= Period

       U(1) = r0 * COS( 2.0D0 * PI * theta)
       U(2) = r0 * SIN( 2.0D0 * PI * theta)

      END SUBROUTINE STPNT
!---------------------------------------------------------------------- 

      SUBROUTINE BCND(NDIM,PAR,ICP,NBC,U0,U1,FB,IJAC,DBC) 
!     ---------- ---- 

      IMPLICIT NONE
      INTEGER, INTENT(IN) :: NDIM,ICP(*),NBC,IJAC
      DOUBLE PRECISION, INTENT(IN) :: PAR(*),U0(NDIM),U1(NDIM)
      DOUBLE PRECISION, INTENT(OUT) :: FB(NBC)
      DOUBLE PRECISION, INTENT(INOUT) :: DBC(NBC,*)

      DOUBLE PRECISION x0, y0, x1, y1, PI, theta, r0, r1

	x0 = U0(1) 
	y0 = U0(2)
	x1 = U1(1) 
	y1 = U1(2) 

        PI = 4*ATAN(1.D0)

	theta = PAR(1)
	r0  = PAR(3)
	r1  = PAR(4)

       	FB(1) = x0 - r0 * COS( 2.0 * PI * theta) 
	FB(2) = y0 - r0 * SIN( 2.0 * PI * theta) 
        FB(3) = SQRT(x1**2 + y1**2) - r1 

      END SUBROUTINE BCND
!---------------------------------------------------------------------- 
      SUBROUTINE ICND
      END SUBROUTINE ICND

      SUBROUTINE FOPT 
      END SUBROUTINE FOPT

      SUBROUTINE PVLS
      END SUBROUTINE PVLS
!---------------------------------------------------------------------- 
