# PoseidonOS Meta Filesystem Layer
Meta filesystem provides a simple file interface that allows to easily manage metadata owned by dedicated module and provides persistency of all metadata being stored underlying meta storage subsystem.

## Contributors
* Shivam (shivam.1@samsung.com)
* Wonchul Lee (wonchul08.lee@samsung.com)    
* Munseop Lim (munseop.lim@samsung.com)

## Design Document (Interal Only)
Please refer to link below. 
- Wiki link: http://globalwiki.itplatform.sec.samsung.net:8099/display/ibof/03_MetaFileSystem
  - You may be required the access permission to our internal wiki system.
- EDM link: https://edm2.sec.samsung.net/cc/link/verLink/155919418107504744/1

## Overview
Meta filesystem layer consists of two parts, Meta filesystem and Meta storage subsystem. 

Meta filesystem services front-end file and system management APIs and handles all aspects of file management as well as its associated metadata such as i-node entries. Meta filesystem internally consists of several functional blocks as following:

- MAI (Meta filesystem Application Interface)
  - This block provides user-level interfaces.
  - It allows you to perform meta filesystem creation and mount/unmount as well as meta file creation/open and file I/O.

- MSC (Meta filesystem Service Core)
  - This block manages entire meta file system focusing on its serviceability.
  - It handles aspect of filesystem core and manages filesystem state in order to coordinate internal modules across all functional blocks.

- MVM (Meta Volume Management)
  - This block provides a full functionality for internal filesystem meta management as a concept of Meta Volume.
  - User can create file and manage it via this block.
  - All Meta file and associated special metadata would be stored and be loaded from associated Meta Volume depending on physical storage type being provided through the initialization path of Meta filesystem.

- MIM (Meta I/O Management)
  - This block provides file accessibility to user. User can send file read/write request through this block.

- MDI (Meta Data Integrity)
  - This block provides end-to-end data protection mechanism and reliability of data presence for each meta page stored in Meta storage subsystem, meaning that it handles data integrity check and its generation for each page.
  - Data integrity feature would be CRC, ECC, data mirroring with CRC and physical journaling
  - Data Integrity feature is configurable by user and it can be determined when the file gets created. For flexibility, user can config this feature at runtime along with file format operation.

- MR (Meta Recovery)
  - This block is responsible for filesystem recovery.
  - Depending on error condition, it recovers Meta Filesystem to put it in right place.
- MCM (Meta Cache Management)
  - This block helps to access meta data even faster by adapting reasonable size of data cache pool.

On the other hand, Meta storage subsystem provides page-level storage interface to Meta filesystem and process physical i/o interacting with PoseidonOS i/o path. Meta filesystem provides two type of data store called **Vstore (Virtual data store)**, **Pstore (Persistent data store)**.

**Vstore** is a fake module of meta storage subsystem that stores all given data into Ramdisk instead of physical media like NV-RAM or SSD array. The purpose of Vstore is to provide quick thurnaround of new feature and can validate functionality without actual physical media. Also, with vstore, we can archive strong test capability by decoupling meta filesystem and meta storage subsystem.

**Pstore** is a production level code of Meta storage subsystem in order to provide persistency of all metadata which basically managed by Meta filesystem.

## Directory Structuring
```
.
├── config
├── include
├── log
├── api_base
├── mai
├── mdi
├── mim
├── msc
├── mvm
├── script
├── storage
├── testdouble
├── unit_test
└── util
```

## Build Description for Standalone Unit Test
### Testing for Target code build
To test production-level code build, please run @ ibof_top:
``` 
make -j8
```
Note that you won't get **mfs** binary due to missing of **main()** function.

### Unit test, Integration test build & Run
To build for top-level functionality test of meta filesystem with pstore, please run:
Do not support`
```

To build for full functionality testing of meta filesystem but with vstore, please run:
```
make vmfs=1 ut=mfs -j8; 
(for clean, make ut=mfs clean)
sudo ./mfs
(for gtest : sudo ./mfs --gtest_filter=*TCName)
```

In order to test each function block of Meta filesystem layer or to test all-at-once, please run:
Do not support 
```
Note that virtual meta filesysystem is used for all function block-level unit tests

## Pre-checkin Test Guideline
All code has to be fully verified and validated to avoid break anything unexpectedly. To ensure this, please follow instruction below:
```
# Please make sure all builds complete successfully without any warning and error notification
Do not support

# Please make sure that there's no any particular memory issue seen. 
Do not support

## MetaFS Test Double Suite
Please refer to README in testdouble folder

