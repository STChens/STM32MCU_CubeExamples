/*
 * Copyright (c) 2018 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/* This header file is included from linker scatter file as well, where only a
 * limited C constructs are allowed. Therefore it is not possible to include
 * here the platform_retarget.h to access flash related defines. To resolve this
 * some of the values are redefined here with different names, these are marked
 * with comment.
 */

/* Flash layout configuration : begin ****************************************/
#define MCUBOOT_OVERWRITE_ONLY     /* Defined: the FW installation uses overwrite method.
                                      UnDefined: The FW installation uses swap mode. */

#define MCUBOOT_EXT_LOADER         /* Defined: Use system bootloader (in system flash).
                                               To enter it, press user button at reset.
                                      Undefined: Do not use system bootloader. */

#define MCUBOOT_APP_IMAGE_NUMBER 1 /* 1: S and NS application binaries are assembled in one single image.
                                      2: Two separated images for S and NS application binaries. */

#define MCUBOOT_S_DATA_IMAGE_NUMBER 0   /* 1: S data image for S application.
                                           0: No S data image. */

#define MCUBOOT_NS_DATA_IMAGE_NUMBER 0  /* 1: NS data image for NS application.
                                           0: No NS data image. */

/*#define DEVICE_1M_FLASH_ENABLE */  /*Defined: the project is for 1M FLASH device
                                       Undefined: the project is for 2M FLASH device */
/* Flash layout configuration : end ******************************************/
#if (MCUBOOT_APP_IMAGE_NUMBER != 1)
#error "Image number must be 1 because App is secure only!"
#endif

/* Total number of images */
#define MCUBOOT_IMAGE_NUMBER (MCUBOOT_APP_IMAGE_NUMBER + MCUBOOT_S_DATA_IMAGE_NUMBER + MCUBOOT_NS_DATA_IMAGE_NUMBER)

/* Use image hash reference to reduce boot time (signature check bypass) */
#define MCUBOOT_USE_HASH_REF

/* control configuration */

/* The size of a partition. This should be large enough to contain a S or NS
 * sw binary. Each FLASH_AREA_IMAGE contains two partitions. See Flash layout
 * above.
 */
#define LOADER_FLASH_DEV_NAME             Driver_FLASH0

/* Flash layout info for BL2 bootloader */
#define FLASH_AREA_IMAGE_SECTOR_SIZE    (0x2000)     /* 8 KB */
#define FLASH_AREA_WRP_GROUP_SIZE       (0x8000)     /* 32 KB */
#if defined(DEVICE_1M_FLASH_ENABLE)
#define FLASH_B_SIZE                    (0x40000)   /* 256 KBytes*/
#else
#define FLASH_B_SIZE                    (0x40000) /* 256 MBytes */
#endif /* DEVICE_1M_FLASH_ENABLE */
#define FLASH_TOTAL_SIZE                (FLASH_B_SIZE+FLASH_B_SIZE) /* 512 KBytes*/
#define FLASH_BASE_ADDRESS              (0x08000000)

/* Flash area IDs */
#define FLASH_AREA_0_ID                 (1)

#define FLASH_AREA_2_ID                 (3)

#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
#define FLASH_AREA_4_ID                 (5)
#endif /* MCUBOOT_S_DATA_IMAGE_NUMBER == 1 */

#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
#define FLASH_AREA_6_ID                 (7)
#endif /* MCUBOOT_S_DATA_IMAGE_NUMBER == 1 */

#define FLASH_AREA_SCRATCH_ID           (9)

/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_0 and IMAGE_1, SCRATCH
 * is used as a temporary storage during image swapping.
 */

/* area for BL2 code protected by hdp */
#define FLASH_AREA_BL2_OFFSET           (0x0000)
#define FLASH_AREA_BL2_SIZE             (0x18000)

/* scratch area */
#if defined(FLASH_AREA_SCRATCH_ID)
#define FLASH_AREA_SCRATCH_DEVICE_ID    (FLASH_DEVICE_ID - FLASH_DEVICE_ID)
#define FLASH_AREA_SCRATCH_OFFSET       (FLASH_AREA_BL2_SIZE)
#if defined(MCUBOOT_OVERWRITE_ONLY)
#define FLASH_AREA_SCRATCH_SIZE         (0x0000) /* Not used in MCUBOOT_OVERWRITE_ONLY mode */
#else
#define FLASH_AREA_SCRATCH_SIZE         (0x10000) /* 64 KB */
#endif
/* control scratch area */
#if (FLASH_AREA_SCRATCH_OFFSET % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_SCRATCH_OFFSET not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /* (FLASH_AREA_SCRATCH_OFFSET % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0*/
#else /* FLASH_AREA_SCRATCH_ID */
#define FLASH_AREA_SCRATCH_SIZE         (0x0)
#endif /* FLASH_AREA_SCRATCH_ID */

/* HDP area end at this address */
#define FLASH_BL2_HDP_END               (FLASH_AREA_SCRATCH_OFFSET+FLASH_AREA_SCRATCH_SIZE-1)
/* control area for BL2 code protected by hdp */
#if ((FLASH_AREA_BL2_OFFSET+FLASH_AREA_BL2_SIZE) % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "HDP area must be aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /* ((FLASH_AREA_BL2_OFFSET+FLASH_AREA_BL2_SIZE) % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */

/* control area under WRP group protection */
#if (FLASH_AREA_BL2_OFFSET % FLASH_AREA_WRP_GROUP_SIZE) != 0
#error "FLASH_AREA_BL2_OFFSET not aligned on FLASH_AREA_WRP_GROUP_SIZE"
#endif /* (FLASH_AREA_BL2_OFFSET % FLASH_AREA_WRP_GROUP_SIZE) != 0 */
#if ((FLASH_AREA_BL2_OFFSET+FLASH_AREA_BL2_SIZE) % FLASH_AREA_WRP_GROUP_SIZE) != 0
#error "(FLASH_AREA_BL2_OFFSET+FLASH_AREA_BL2_SIZE) not aligned on FLASH_AREA_WRP_GROUP_SIZE"
#endif /* ((FLASH_AREA_BL2_OFFSET+FLASH_AREA_BL2_SIZE) % FLASH_AREA_WRP_GROUP_SIZE) != 0 */

/* BL2 partitions size */
#define FLASH_S_PARTITION_SIZE          (FLASH_TOTAL_SIZE - FLASH_AREA_0_OFFSET) /* 512 KB for S partition */
#define FLASH_NS_PARTITION_SIZE         (0x0) /* 0 KB for NS partition */
#define FLASH_PARTITION_SIZE            (FLASH_S_PARTITION_SIZE+FLASH_NS_PARTITION_SIZE)

/* App FW slot size. 
This value may change if the layout changes, in such case, please recalculate the size using the xlsx file */
#define FLASH_S_ACTIVESLOT_SIZE         (0x2A000) /* 168 KB for Code slot of secure only app. */

#define FLASH_MAX_APP_PARTITION_SIZE    FLASH_PARTITION_SIZE
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
#define FLASH_S_DATA_PARTITION_SIZE     (FLASH_AREA_IMAGE_SECTOR_SIZE)
#else
#define FLASH_S_DATA_PARTITION_SIZE     (0x0)
#endif /* (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */

#define FLASH_MAX_DATA_PARTITION_SIZE   (FLASH_S_DATA_PARTITION_SIZE)
#define FLASH_MAX_PARTITION_SIZE        ((FLASH_MAX_APP_PARTITION_SIZE >   \
                                         FLASH_MAX_DATA_PARTITION_SIZE) ? \
                                         FLASH_MAX_APP_PARTITION_SIZE : \
                                         FLASH_MAX_DATA_PARTITION_SIZE)

/* BL2 flash areas */
#define FLASH_AREA_BEGIN_OFFSET         (FLASH_AREA_SCRATCH_SIZE+FLASH_AREA_BL2_SIZE)
#define FLASH_AREAS_DEVICE_ID           (FLASH_DEVICE_ID - FLASH_DEVICE_ID)

/* FLASH_AREA_BEGIN_OFFSET :
   Before FLASH_AREA_BEGIN_OFFSET is BL2 and Scratch area
   Starting from FLASH_AREA_BEGIN_OFFSET is S data area, S code active slot and so on
   Basically, all content that does not belong to Secure boot or managed by secure boot is starting from FLASH_AREA_BEGIN_OFFSET
*/

/* For SECURE ONLY APP use case, the possible slots following FLASH_AREA_BEGIN_OFFSET are
   +----------+--------------------------------+-------------------------------+
   | Area ID  |  Purpose                       |   Size                        |                 
   +----------+--------------------------------+-------------------------------+
   | 4        |  S data active slot            |   FLASH_S_DATA_PARTITION_SIZE |                 
   +----------+--------------------------------+-------------------------------+
   | 0        |  S App data active slot        |   FLASH_S_ACTIVESLOT_SIZE     |                 
   +----------+--------------------------------+-------------------------------+
   | 2        |  S App data download slot      |   FLASH_S_ACTIVESLOT_SIZE     |                 
   +----------+--------------------------------+-------------------------------+
   | 6        |  S data download slot          |   FLASH_S_DATA_PARTITION_SIZE |                 
   +----------+--------------------------------+-------------------------------+
   
*/

/* Secure data image primary slot */
#if defined (FLASH_AREA_4_ID)
#define FLASH_AREA_4_DEVICE_ID          (FLASH_AREAS_DEVICE_ID)
#define FLASH_AREA_4_OFFSET             (FLASH_AREA_BEGIN_OFFSET)
#define FLASH_AREA_4_SIZE               (FLASH_S_DATA_PARTITION_SIZE)
/* Control Secure data image primary slot */
#if (FLASH_AREA_4_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_4_OFFSET  not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /* (FLASH_AREA_4_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */
#else /* FLASH_AREA_4_ID */
#define FLASH_AREA_4_OFFSET             (0x0)
#define FLASH_AREA_4_SIZE               (0x0)
#endif /* FLASH_AREA_4_ID */

/* External loader area */


/* Secure app image primary slot */
#if defined(FLASH_AREA_0_ID)
#define FLASH_AREA_0_DEVICE_ID          (FLASH_AREAS_DEVICE_ID)
#define FLASH_AREA_0_OFFSET             (FLASH_AREA_BEGIN_OFFSET + FLASH_AREA_4_SIZE)
#if (MCUBOOT_APP_IMAGE_NUMBER == 2)
#define FLASH_AREA_0_SIZE               (FLASH_S_ACTIVESLOT_SIZE)
#else
#define FLASH_AREA_0_SIZE               (FLASH_S_ACTIVESLOT_SIZE)
#endif /* (MCUBOOT_APP_IMAGE_NUMBER == 2) */
/* Control Secure app image primary slot */
#if (FLASH_AREA_0_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_0_OFFSET  not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /*  (FLASH_AREA_0_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */
#else /* FLASH_AREA_0_ID */
#define FLASH_AREA_0_OFFSET             (0x0)
#define FLASH_AREA_0_SIZE               (0x0)
#endif /* FLASH_AREA_0_ID */


/* Secure app image secondary slot */
#if defined(FLASH_AREA_2_ID)
#define FLASH_AREA_2_DEVICE_ID          (FLASH_AREAS_DEVICE_ID)
#define FLASH_AREA_2_OFFSET             (FLASH_AREA_BEGIN_OFFSET + FLASH_AREA_4_SIZE + \
                                         FLASH_AREA_0_SIZE )
#define FLASH_AREA_2_SIZE               (FLASH_S_ACTIVESLOT_SIZE)
/* Control Secure app image secondary slot */
#if (FLASH_AREA_2_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_2_OFFSET  not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /*   (FLASH_AREA_2_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */
#else /* FLASH_AREA_2_ID */
#define FLASH_AREA_2_OFFSET             (0x0)
#define FLASH_AREA_2_SIZE               (0x0)
#endif /* FLASH_AREA_2_ID */


/* Secure data image secondary slot */
#if defined(FLASH_AREA_6_ID)
#define FLASH_AREA_6_DEVICE_ID          (FLASH_AREAS_DEVICE_ID)
#define FLASH_AREA_6_OFFSET             (FLASH_AREA_BEGIN_OFFSET + FLASH_AREA_4_SIZE + \
                                         FLASH_AREA_0_SIZE + FLASH_AREA_2_SIZE)
#define FLASH_AREA_6_SIZE               (FLASH_S_DATA_PARTITION_SIZE)
/* Control Secure data image secondary slot */
#if (FLASH_AREA_6_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_6_OFFSET  not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /*  (FLASH_AREA_6_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */
#else /* FLASH_AREA_6_ID */
#define FLASH_AREA_6_OFFSET             (0x0)
#define FLASH_AREA_6_SIZE               (0x0)
#endif /* FLASH_AREA_6_ID */

/* flash areas end offset */
#define FLASH_AREA_END_OFFSET           (FLASH_AREA_BEGIN_OFFSET + FLASH_AREA_4_SIZE + \
                                         FLASH_AREA_0_SIZE + FLASH_AREA_2_SIZE + \
                                         FLASH_AREA_6_SIZE)
/* Control flash area end */
#if (FLASH_AREA_END_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0
#error "FLASH_AREA_END_OFFSET  not aligned on FLASH_AREA_IMAGE_SECTOR_SIZE"
#endif /*  (FLASH_AREA_END_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */

/* EDATA area */
#define FLASH_AREA_EDATA_SIZE    (0x2000) /* 8KB at the end of the flash is reserved for EDATA */
#define FLASH_AREA_EDATA_OFFSET  (FLASH_TOTAL_SIZE - FLASH_AREA_EDATA_SIZE) 

/* User loader area */
#if (defined MCUBOOT_EXT_LOADER && !defined USE_SYTEM_BOOTLOADER)

#define LOADER_FLASH_DEV_NAME             Driver_FLASH0

#define FLASH_AREA_LOADER_SIZE   (0xC000) /* 48 KB for loader */
#define FLASH_AREA_LOADER_OFFSET (FLASH_AREA_EDATA_OFFSET - FLASH_AREA_LOADER_SIZE) /* put at the end of the flash before EDATA area */
#else /*  (defined MCUBOOT_EXT_LOADER && !defined USE_SYTEM_BOOTLOADER) */
#define FLASH_AREA_LOADER_SIZE   (0) /* 48 KB for loader */
#define FLASH_AREA_LOADER_OFFSET (FLASH_AREA_EDATA_OFFSET - FLASH_AREA_LOADER_SIZE) /* put at the end of the flash before EDATA area */
#endif /*  (defined MCUBOOT_EXT_LOADER && !defined USE_SYTEM_BOOTLOADER) */

/* Control flash area end and loader offset */
#if (FLASH_AREA_END_OFFSET  >=  FLASH_AREA_LOADER_OFFSET) 
#error "FLASH_AREA_END_OFFSET overlap with FLASH_AREA_LOADER_OFFSET!"
#endif /*  (FLASH_AREA_END_OFFSET  % FLASH_AREA_IMAGE_SECTOR_SIZE) != 0 */

/*
 * The maximum number of status entries supported by the bootloader.
 */
#if defined(MCUBOOT_OVERWRITE_ONLY)
#define MCUBOOT_STATUS_MAX_ENTRIES        (0)
#else /* not MCUBOOT_OVERWRITE_ONLY */
#define MCUBOOT_STATUS_MAX_ENTRIES        (((FLASH_MAX_PARTITION_SIZE - 1) / \
                                            FLASH_AREA_SCRATCH_SIZE) + 1)
#endif /* MCUBOOT_OVERWRITE_ONLY */
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS           ((FLASH_MAX_PARTITION_SIZE) / \
                                           FLASH_AREA_IMAGE_SECTOR_SIZE)

#define SECURE_IMAGE_OFFSET             (0x0)
#define SECURE_IMAGE_MAX_SIZE           FLASH_S_ACTIVESLOT_SIZE

/* Flash device name used by BL2 and NV Counter
 * Name is defined in flash driver file: low_level_flash.c
 */
#if !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_APP_IMAGE_NUMBER == 2)
/* Flash Driver Used to Confirm Secure App Image */
#define  FLASH_PRIMARY_SECURE_DEV_NAME             Driver_FLASH0
#endif /* !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_APP_IMAGE_NUMBER == 2) */
/* Flash Driver Used to Confirm NonSecure App Image or MCUBOOT_APP_IMAGE_NUMBER = 1 */
#define  FLASH_PRIMARY_NONSECURE_DEV_NAME          Driver_FLASH0
#if !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
/* Flash Driver Used to Confirm Secure Data Image */
#define  FLASH_PRIMARY_DATA_SECURE_DEV_NAME        Driver_FLASH0
#endif /* !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#if !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
/* Flash Driver Used to Confirm NonSecure Data Image */
#define  FLASH_PRIMARY_DATA_NONSECURE_DEV_NAME     Driver_FLASH0
#endif /* !defined(MCUBOOT_OVERWRITE_ONLY) && (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#define FLASH_DEV_NAME                             Driver_FLASH0


/* BL2 NV Counters definitions  */
#define BL2_NV_COUNTERS_AREA_ADDR        FLASH_BL2_NVCNT_AREA_OFFSET
#define BL2_NV_COUNTERS_AREA_SIZE        FLASH_BL2_NVCNT_AREA_SIZE

/* OBK */
#define OBK_HDPL0_OFFSET        (0x00U)         /* First OBkey Hdpl 0 */
#define OBK_HDPL0_END           (0xFFU)         /* Last OBKey Hdpl 0 */
#define OBK_HDPL1_OFFSET        (0x100U)        /* First OBkey Hdpl 1 */
#define OBK_HDPL1_END           (0x8FFU)        /* Last OBKey Hdpl 1 */
#define OBK_HDPL2_OFFSET        (0x900U)        /* First OBkey Hdpl 2 */
#define OBK_HDPL2_END           (0xBFFU)        /* Last OBKey Hdpl 2 */
#define OBK_HDPL3_OFFSET        (0xC00U)        /* First OBkey Hdpl 3 */
#define OBK_HDPL3_END           (0x1FFFU)       /* Last OBKey Hdpl 3 */

/* Engi bits */
#define ENGI_BASE_NS                        (0x08FFF800U)
#define ENGI_SIZE                           (0x40U)

/* Systeme Flash description */
#define RSS_LIB_BASE                        (0x0FF94000U)
#define RSS_LIB_SIZE                        (0x2000U)
#define BOOTLOADER_BASE_NS                  (0x0BF97000U)
#define BOOTLOADER_SIZE                     (0x9400U)
#define STM32_DESCRIPTOR_BASE_NS_3          (0x0BF9FB00U)
#define STM32_DESCRIPTOR_BASE_NS_2          (0x0BF9FD00U)
#define STM32_DESCRIPTOR_BASE_NS_1          (0x0BF9FE00U)
#define RSSLIB_PFUNC_3                      (0x0BF9FB68UL)
#define RSSLIB_PFUNC_2                      (0x0BF9FD68UL)
#define RSSLIB_PFUNC_1                      (0x0BF9FE68UL)
#define STM32_DESCRIPTOR_SIZE               (0x100U)
#define STM32_DESCRIPTOR_BASE_NS            (STM32_DESCRIPTOR_BASE_NS_3) /* use for mpu region the lowest address*/
#define STM32_DESCRIPTOR_END_NS             (STM32_DESCRIPTOR_BASE_NS_1 + STM32_DESCRIPTOR_SIZE -1) /* to cover all descriptors */

#endif /* __FLASH_LAYOUT_H__ */
