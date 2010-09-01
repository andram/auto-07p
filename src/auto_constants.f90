!------ --------------
! these are the AUTO constants, *exactly* as given in the fort.2 or c. file
! don't use this module from any parallellized function (FUNI and ICNI).
MODULE AUTO_CONSTANTS

  IMPLICIT NONE
  INTEGER NPARX
  INCLUDE 'auto.h'

  INTEGER NDIM,IPS,IRS,ILP
  CHARACTER(13), ALLOCATABLE :: ICU(:)
  INTEGER NTST,NCOL,IAD,ISP,ISW,IPLT,NBC,NINT
  INTEGER NMX
  DOUBLE PRECISION RL0,RL1,A0,A1
  INTEGER NPR,MXBF,IID,ITMX,ITNW,NWTN,JAC
  DOUBLE PRECISION EPSL,EPSU,EPSS
  DOUBLE PRECISION DS,DSMIN,DSMAX
  INTEGER IADS,NPAR
  TYPE INDEXVAR
     CHARACTER(13) INDEX
     DOUBLE PRECISION VAR
  END TYPE INDEXVAR
  TYPE(INDEXVAR),ALLOCATABLE :: IVTHL(:),IVTHU(:),UVALS(:),PARVALS(:)
  TYPE INDEXMVAR
     CHARACTER(13) INDEX
     DOUBLE PRECISION, POINTER :: VAR(:)
  END TYPE INDEXMVAR
  TYPE(INDEXMVAR),ALLOCATABLE :: IVUZR(:)

  TYPE INDEXSTR
     INTEGER INDEX
     CHARACTER(13) STR
  END TYPE INDEXSTR
  TYPE(INDEXSTR), ALLOCATABLE :: unames(:), parnames(:)
  CHARACTER(13), ALLOCATABLE :: SP(:), STOPS(:)
  CHARACTER(13) :: SIRS
  CHARACTER(256) :: EFILE, SFILE, SVFILE, DATFILE

! HomCont comstants:
  TYPE HCONST_TYPE
     INTEGER NUNSTAB,NSTAB,IEQUIB,ITWIST,ISTART
     INTEGER, POINTER :: IREV(:),IFIXED(:),IPSI(:)
  END TYPE
  TYPE(HCONST_TYPE)HCONST

  LOGICAL, SAVE :: NEWCFILE=.FALSE.

  ! these are the AUTO parameters, as used to be in the IAP and RAP arrays.
  ! NOTE: for MPI:
  ! * SEQUENCE is necessary, first comes double precision, then integer
  ! * DS should be the first double precision variable
  ! * NDIM should be the first integer variable  
  ! * NIAP and NRAP are used!
  ! * For alignment, always keep an even number of integer variables.
  !   (NIAP should be even;  make the structure length a multiple of its
  !    largest element)
  INTEGER, PARAMETER :: NIAP = 36
  INTEGER, PARAMETER :: NRAP = 16
  TYPE AUTOPARAMETERS
     SEQUENCE

     DOUBLE PRECISION DS, DSMIN, DSMAX, RDS, RL0, RL1, A0, A1
     DOUBLE PRECISION EPSL, EPSU, EPSS

     DOUBLE PRECISION DET, FLDF, HBFF, BIFF, SPBF

     INTEGER NDIM, IPS, IRS, ILP, NTST, NCOL, IAD, IADS, ISP, ISW
     INTEGER IPLT, NBC, NINT, NMX, NUZR, NPR, MXBF, IID, ITMX, ITNW
     INTEGER NWTN, JAC, NPAR, NREV

     INTEGER NDM, NPARI, ITDS, ITP, ITPST, NFPR, IBR, NTOT
     INTEGER NINS, LAB, NICP, NTEST

  END TYPE AUTOPARAMETERS

END MODULE AUTO_CONSTANTS
