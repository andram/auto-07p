! dummy file, used in case MPI is not available.

module autompi

use auto_constants, only: autoparameters

implicit none
private
public :: mpiini, mpiiap, mpiwfi, mpicon, mpisbv, mpibcast, mpibcasti
public :: mpibcastap
public :: mpiscat, mpigat, mpiend, mpitim, mpiiam, mpikwt, partition

contains

subroutine mpiini()
end subroutine mpiini

integer function mpiiam()
  mpiiam = 0
end function mpiiam

integer function mpikwt()
  mpikwt = 1
end function mpikwt

subroutine mpiiap(ap)
  type(autoparameters) :: ap
end subroutine mpiiap

logical function mpiwfi(autobv)
  logical :: autobv
  mpiwfi = .false.
end function mpiwfi

subroutine mpicon(s1,a1,a2,bb,cc,c2,d,faa,fc,ntst,nov,ncb,nrc,ifst)
  integer, intent(in) :: ntst, nov, ncb, nrc, ifst
  double precision, intent(inout) :: a1(nov,nov,*),a2(nov,nov,*),bb(ncb,nov,*)
  double precision, intent(inout) :: cc(nov,nrc,*),c2(nov,nrc,*)
  double precision, intent(inout) :: s1(nov,nov,*),d(ncb,*),faa(nov,*),fc(*)
end subroutine mpicon

subroutine mpisbv(ap,par,icp,nra,ups,uoldps,udotps,upoldp,dtm, &
     thu,ifst,nllv)
  type(autoparameters) :: ap
  integer, intent(in) :: nra,icp(*)
  integer, intent(inout) :: ifst,nllv
  double precision :: par(*),dtm(*),thu(*)
  double precision :: ups(nra,*),uoldps(nra,*),udotps(nra,*),upoldp(nra,*)
end subroutine mpisbv

subroutine mpibcast(buf,len)
  integer, intent(in) :: len
  double precision, intent(inout) :: buf(len)
end subroutine mpibcast

subroutine mpibcasti(buf,len)
  integer, intent(in) :: len
  integer, intent(inout) :: buf(len)
end subroutine mpibcasti

subroutine mpibcastap(ap)
  type(autoparameters), intent(inout) :: ap
end subroutine mpibcastap

subroutine mpiscat(buf,ndx,n,add)
  integer, intent(in) :: ndx,n,add
  double precision, intent(inout) :: buf(ndx,*)
end subroutine mpiscat

subroutine mpigat(buf,ndx,n)
  integer, intent(in) :: ndx,n
  double precision, intent(inout) :: buf(ndx,*)
end subroutine mpigat

subroutine mpiend()
end subroutine mpiend

subroutine mpitim(tim)
  double precision tim
end subroutine mpitim  

subroutine partition(n,kwt,m)
  integer n,kwt,m(kwt)
  m(1) = n
end subroutine partition

end module autompi
