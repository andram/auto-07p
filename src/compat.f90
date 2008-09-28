module compat

implicit none
public :: autoflush, autim

contains

! This file calls no extensions (see f2003.f90 for the new Standard forms)
! You can edit it to adjust these routines to the compiler that is used.

  subroutine autoflush(i)
    integer, intent(in) :: i
    !call flush(i)
  end subroutine autoflush

!-----------------------------------------------------------------------
!          Timing AUTO
!-----------------------------------------------------------------------

! ------ --------- --------
  double precision function autim()

    !call cpu_time(etime)
    !autim = etime
    autim = -1

  end function autim

end module compat
