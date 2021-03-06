/**
 * @file attr_table.c
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <string.h>

#include "attr_validator.h"
#include "attr_custom_validator.h"
#include "attr_table.h"

/* clang-format off */

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define DRW DEFAULT_RW_ATTRIBUTE_VALUES
#define DRO DEFAULT_RO_ATTRIBUTE_VALUES

/* Add things to the end of the API document.
 * Do not remove items. Change them to deprecated.
 */
typedef struct rw_attribute {
	/* pystart - rw attributes */
	char location[32 + 1];
	bool lock;
	char resetReason[12 + 1];
	int8_t txPower;
	uint16_t networkId;
	uint8_t configVersion;
	uint8_t hardwareVersion;
	uint32_t qrtcLastSet;
	bool commissioned;
	char rootCaName[48 + 1];
	char clientCertName[48 + 1];
	char clientKeyName[48 + 1];
	char endpoint[254 + 1];
	char port[16 + 1];
	char clientId[32 + 1];
	char topicPrefix[32 + 1];
	uint16_t batteryLowThreshold;
	uint16_t batteryAlarmThreshold;
	uint16_t battery4;
	uint16_t battery3;
	uint16_t battery2;
	uint16_t battery1;
	uint16_t battery0;
	uint8_t motionOdr;
	uint8_t motionThresh;
	uint8_t motionScale;
	uint8_t motionDuration;
	uint8_t sdLogMaxSize;
	uint8_t ctAesKey[16];
	uint32_t joinDelay;
	uint16_t joinMin;
	uint16_t joinMax;
	uint32_t joinInterval;
	bool delayCloudReconnect;
	uint32_t modemDesiredLogLevel;
	char loadPath[32 + 1];
	char dumpPath[32 + 1];
	bool nvImported;
	float floaty;
	uint8_t lwm2mPsk[16];
	char lwm2mClientId[32 + 1];
	char lwm2mPeerUrl[128 + 1];
	/* pyend */
} rw_attribute_t;

static const rw_attribute_t DEFAULT_RW_ATTRIBUTE_VALUES = {
	/* pystart - rw defaults */
	.location = "",
	.lock = false,
	.resetReason = "RESETPIN",
	.txPower = 0,
	.networkId = 0,
	.configVersion = 0,
	.hardwareVersion = 0,
	.qrtcLastSet = 0,
	.commissioned = false,
	.rootCaName = "/lfs/root_ca.pem",
	.clientCertName = "/lfs/client_cert.pem",
	.clientKeyName = "/lfs/client_key.pem",
	.endpoint = "a3273rvo818l4w-ats.iot.us-east-1.amazonaws.com",
	.port = "8883",
	.clientId = "",
	.topicPrefix = "mg100-ct/dev/gw/",
	.batteryLowThreshold = 3400,
	.batteryAlarmThreshold = 3000,
	.battery4 = 4200,
	.battery3 = 3800,
	.battery2 = 3400,
	.battery1 = 3000,
	.battery0 = 2750,
	.motionOdr = 5,
	.motionThresh = 10,
	.motionScale = 2,
	.motionDuration = 6,
	.sdLogMaxSize = 0,
	.ctAesKey = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	.joinDelay = 0,
	.joinMin = 1,
	.joinMax = 100,
	.joinInterval = 1,
	.delayCloudReconnect = false,
	.modemDesiredLogLevel = 1,
	.loadPath = "/lfs/params.txt",
	.dumpPath = "/lfs/dump.txt",
	.nvImported = false,
	.floaty = 0.13,
	.lwm2mPsk = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
	.lwm2mClientId = "Client_identity",
	.lwm2mPeerUrl = "uwterminalx.lairdconnect.com"
	/* pyend */
};

typedef struct ro_attribute {
	/* pystart - ro attributes */
	char firmwareVersion[11 + 1];
	char bluetoothAddress[12 + 1];
	uint32_t resetCount;
	int64_t upTime;
	uint16_t batteryVoltageMv;
	char attributeVersion[11 + 1];
	uint32_t qrtc;
	char name[32 + 1];
	char board[32 + 1];
	char buildId[64 + 1];
	char appType[32 + 1];
	char mount[32 + 1];
	enum cert_status certStatus;
	enum gateway_state gatewayState;
	uint8_t batteryCapacity;
	int16_t batteryTemperature;
	uint8_t batteryChargingState;
	bool batteryAlarm;
	bool motionAlarm;
	float powerSupplyVoltage;
	char gatewayId[15 + 1];
	enum lte_network_state lteNetworkState;
	enum lte_startup_state lteStartupState;
	int16_t lteRsrp;
	int16_t lteSinr;
	enum lte_sleep_state lteSleepState;
	uint8_t lteRat;
	char iccid[20 + 1];
	char lteSerialNumber[14 + 1];
	char lteVersion[29 + 1];
	char bands[20 + 1];
	char activeBands[20 + 1];
	enum central_state centralState;
	char sensorBluetoothAddress[30 + 1];
	enum modem_boot modemBoot;
	char apn[64 + 1];
	char apnUsername[65 + 1];
	char apnPassword[65 + 1];
	uint8_t apnControlPoint;
	int32_t apnStatus;
	uint8_t lteOperatorIndex;
	enum fota_control_point fotaControlPoint;
	enum fota_status fotaStatus;
	char fotaFileName[64 + 1];
	uint32_t fotaSize;
	uint32_t fotaCount;
	enum generate_psk generatePsk;
	enum lte_init_error lteInitError;
	enum cloud_error cloudError;
	bool commissioningBusy;
	char imsi[15 + 1];
	enum modem_functionality modemFunctionality;
	/* pyend */
} ro_attribute_t;

static const ro_attribute_t DEFAULT_RO_ATTRIBUTE_VALUES = {
	/* pystart - ro defaults */
	.firmwareVersion = "0.0.0",
	.bluetoothAddress = "0",
	.resetCount = 0,
	.upTime = 0,
	.batteryVoltageMv = 0,
	.attributeVersion = "0.4.20",
	.qrtc = 0,
	.name = "",
	.board = "",
	.buildId = "0",
	.appType = "",
	.mount = "/lfs",
	.certStatus = 0,
	.gatewayState = 0,
	.batteryCapacity = 0,
	.batteryTemperature = 0,
	.batteryChargingState = 0,
	.batteryAlarm = false,
	.motionAlarm = false,
	.powerSupplyVoltage = 0,
	.gatewayId = "",
	.lteNetworkState = 0,
	.lteStartupState = 0,
	.lteRsrp = 0,
	.lteSinr = 0,
	.lteSleepState = 0,
	.lteRat = 0,
	.iccid = "",
	.lteSerialNumber = "",
	.lteVersion = "",
	.bands = "",
	.activeBands = "",
	.centralState = 0,
	.sensorBluetoothAddress = "",
	.modemBoot = 0,
	.apn = "",
	.apnUsername = "",
	.apnPassword = "",
	.apnControlPoint = 0,
	.apnStatus = 0,
	.lteOperatorIndex = 255,
	.fotaControlPoint = 0,
	.fotaStatus = 0,
	.fotaFileName = "",
	.fotaSize = 0,
	.fotaCount = 0,
	.generatePsk = 0,
	.lteInitError = 0,
	.cloudError = 0,
	.commissioningBusy = false,
	.imsi = "",
	.modemFunctionality = 0
	/* pyend */
};

/* pystart - remap */
#define attr_get_string_certStatus          attr_get_string_cert_status
#define attr_get_string_gatewayState        attr_get_string_gateway_state
#define attr_get_string_lteNetworkState     attr_get_string_lte_network_state
#define attr_get_string_lteStartupState     attr_get_string_lte_startup_state
#define attr_get_string_lteSleepState       attr_get_string_lte_sleep_state
#define attr_get_string_centralState        attr_get_string_central_state
#define attr_get_string_modemBoot           attr_get_string_modem_boot
#define attr_get_string_fotaControlPoint    attr_get_string_fota_control_point
#define attr_get_string_fotaStatus          attr_get_string_fota_status
#define attr_get_string_generatePsk         attr_get_string_generate_psk
#define attr_get_string_lteInitError        attr_get_string_lte_init_error
#define attr_get_string_cloudError          attr_get_string_cloud_error
#define attr_get_string_modemFunctionality  attr_get_string_modem_functionality
/* pyend */

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static rw_attribute_t rw;
static ro_attribute_t ro;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/**
 * @brief Table shorthand
 *
 * @ref CreateStruct (Python script)
 * Writable but non-savable values are populated using RO macro.
 *
 *.........name...value...default....size...writable..readable..get enum str
 */
#define RW_ATTRS(n) STRINGIFY(n), rw.n, DRW.n, sizeof(rw.n), NULL
#define RW_ATTRX(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n), NULL
#define RW_ATTRE(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n), attr_get_string_ ## n
#define RO_ATTRS(n) STRINGIFY(n), ro.n, DRO.n, sizeof(ro.n), NULL
#define RO_ATTRX(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n), NULL
#define RO_ATTRE(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n), attr_get_string_ ## n

#define y true
#define n false

/* If min == max then range isn't checked. */

/* index....id.name.....................type.savable.writable.readable.lockable.broadcast.deprecated.validator..min.max. */
const struct attr_table_entry ATTR_TABLE[ATTR_TABLE_SIZE] = {
	/* pystart - attribute table */
	[0  ] = { 1  , RW_ATTRS(location)                      , ATTR_TYPE_STRING        , y, y, y, y, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[1  ] = { 5  , RW_ATTRX(lock)                          , ATTR_TYPE_BOOL          , y, y, y, y, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[2  ] = { 11 , RO_ATTRS(firmwareVersion)               , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 3         , .max.ux = 11         },
	[3  ] = { 12 , RW_ATTRS(resetReason)                   , ATTR_TYPE_STRING        , y, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 12         },
	[4  ] = { 13 , RO_ATTRS(bluetoothAddress)              , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 12        , .max.ux = 12         },
	[5  ] = { 14 , RO_ATTRX(resetCount)                    , ATTR_TYPE_U32           , n, n, y, n, n, n, av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[6  ] = { 16 , RO_ATTRX(upTime)                        , ATTR_TYPE_S64           , n, n, y, n, n, n, av_int64            , attr_prepare_upTime                 , .min.ux = 0         , .max.ux = 0          },
	[7  ] = { 59 , RW_ATTRX(txPower)                       , ATTR_TYPE_S8            , y, y, y, n, y, n, av_int8             , NULL                                , .min.sx = -40       , .max.sx = 8          },
	[8  ] = { 60 , RW_ATTRX(networkId)                     , ATTR_TYPE_U16           , y, y, y, y, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[9  ] = { 61 , RW_ATTRX(configVersion)                 , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[10 ] = { 63 , RW_ATTRX(hardwareVersion)               , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[11 ] = { 75 , RO_ATTRX(batteryVoltageMv)              , ATTR_TYPE_U16           , n, n, y, n, n, n, av_uint16           , attr_prepare_batteryVoltageMv       , .min.ux = 0         , .max.ux = 0          },
	[12 ] = { 93 , RO_ATTRS(attributeVersion)              , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 6         , .max.ux = 11         },
	[13 ] = { 94 , RO_ATTRX(qrtc)                          , ATTR_TYPE_U32           , n, n, y, n, n, n, av_uint32           , attr_prepare_qrtc                   , .min.ux = 0         , .max.ux = 0          },
	[14 ] = { 95 , RW_ATTRX(qrtcLastSet)                   , ATTR_TYPE_U32           , y, n, y, n, n, n, av_uint32           , attr_prepare_qrtcLastSet            , .min.ux = 0         , .max.ux = 0          },
	[15 ] = { 140, RO_ATTRS(name)                          , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 1         , .max.ux = 32         },
	[16 ] = { 142, RO_ATTRS(board)                         , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 1         , .max.ux = 32         },
	[17 ] = { 143, RO_ATTRS(buildId)                       , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 1         , .max.ux = 64         },
	[18 ] = { 144, RO_ATTRS(appType)                       , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 1         , .max.ux = 32         },
	[19 ] = { 145, RO_ATTRS(mount)                         , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[20 ] = { 146, RW_ATTRX(commissioned)                  , ATTR_TYPE_BOOL          , y, y, y, n, y, n, av_cpb              , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[21 ] = { 147, RO_ATTRE(certStatus)                    , ATTR_TYPE_S8            , n, n, y, n, n, n, av_int8             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[22 ] = { 148, RW_ATTRS(rootCaName)                    , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 48         },
	[23 ] = { 149, RW_ATTRS(clientCertName)                , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 48         },
	[24 ] = { 150, RW_ATTRS(clientKeyName)                 , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 48         },
	[25 ] = { 151, RW_ATTRS(endpoint)                      , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 254        },
	[26 ] = { 152, RW_ATTRS(port)                          , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 4         , .max.ux = 16         },
	[27 ] = { 153, RW_ATTRS(clientId)                      , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[28 ] = { 154, RW_ATTRS(topicPrefix)                   , ATTR_TYPE_STRING        , y, y, y, n, y, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[29 ] = { 155, RO_ATTRE(gatewayState)                  , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[30 ] = { 157, RO_ATTRX(batteryCapacity)               , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 4          },
	[31 ] = { 158, RO_ATTRX(batteryTemperature)            , ATTR_TYPE_S16           , n, n, y, n, n, n, av_int16            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[32 ] = { 159, RO_ATTRX(batteryChargingState)          , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[33 ] = { 160, RW_ATTRX(batteryLowThreshold)           , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[34 ] = { 161, RW_ATTRX(batteryAlarmThreshold)         , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[35 ] = { 162, RW_ATTRX(battery4)                      , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[36 ] = { 163, RW_ATTRX(battery3)                      , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[37 ] = { 164, RW_ATTRX(battery2)                      , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[38 ] = { 165, RW_ATTRX(battery1)                      , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[39 ] = { 166, RW_ATTRX(battery0)                      , ATTR_TYPE_U16           , y, y, y, n, y, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535      },
	[40 ] = { 167, RO_ATTRX(batteryAlarm)                  , ATTR_TYPE_BOOL          , n, n, y, n, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[41 ] = { 168, RW_ATTRX(motionOdr)                     , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[42 ] = { 169, RW_ATTRX(motionThresh)                  , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[43 ] = { 170, RW_ATTRX(motionScale)                   , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[44 ] = { 171, RW_ATTRX(motionDuration)                , ATTR_TYPE_U8            , y, y, y, n, y, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255        },
	[45 ] = { 172, RO_ATTRX(motionAlarm)                   , ATTR_TYPE_BOOL          , n, n, y, n, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[46 ] = { 173, RW_ATTRX(sdLogMaxSize)                  , ATTR_TYPE_U8            , y, y, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[47 ] = { 174, RW_ATTRX(ctAesKey)                      , ATTR_TYPE_BYTE_ARRAY    , y, y, n, n, n, n, av_array            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[48 ] = { 175, RO_ATTRX(powerSupplyVoltage)            , ATTR_TYPE_FLOAT         , n, n, y, n, n, n, av_float            , NULL                                , .min.fx = 0.0       , .max.fx = 10.0       },
	[49 ] = { 176, RO_ATTRS(gatewayId)                     , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 15         },
	[50 ] = { 177, RO_ATTRE(lteNetworkState)               , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[51 ] = { 178, RO_ATTRE(lteStartupState)               , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[52 ] = { 179, RO_ATTRX(lteRsrp)                       , ATTR_TYPE_S16           , n, n, y, n, n, n, av_int16            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[53 ] = { 180, RO_ATTRX(lteSinr)                       , ATTR_TYPE_S16           , n, n, y, n, n, n, av_int16            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[54 ] = { 181, RO_ATTRE(lteSleepState)                 , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[55 ] = { 182, RO_ATTRX(lteRat)                        , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 1          },
	[56 ] = { 183, RO_ATTRS(iccid)                         , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 20         },
	[57 ] = { 184, RO_ATTRS(lteSerialNumber)               , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 14         },
	[58 ] = { 185, RO_ATTRS(lteVersion)                    , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 29         },
	[59 ] = { 186, RO_ATTRS(bands)                         , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 20        , .max.ux = 20         },
	[60 ] = { 187, RO_ATTRS(activeBands)                   , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 20        , .max.ux = 20         },
	[61 ] = { 188, RO_ATTRE(centralState)                  , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[62 ] = { 189, RO_ATTRS(sensorBluetoothAddress)        , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 30         },
	[63 ] = { 190, RW_ATTRX(joinDelay)                     , ATTR_TYPE_U32           , y, y, y, n, y, n, av_cp32             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[64 ] = { 191, RW_ATTRX(joinMin)                       , ATTR_TYPE_U16           , y, y, y, n, n, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[65 ] = { 192, RW_ATTRX(joinMax)                       , ATTR_TYPE_U16           , y, y, y, n, n, n, av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[66 ] = { 193, RW_ATTRX(joinInterval)                  , ATTR_TYPE_U32           , y, y, y, n, n, n, av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[67 ] = { 194, RO_ATTRE(modemBoot)                     , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , attr_prepare_modemBoot              , .min.ux = 0         , .max.ux = 0          },
	[68 ] = { 195, RW_ATTRX(delayCloudReconnect)           , ATTR_TYPE_BOOL          , y, y, y, n, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[69 ] = { 196, RO_ATTRS(apn)                           , ATTR_TYPE_STRING        , n, y, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 64         },
	[70 ] = { 197, RO_ATTRS(apnUsername)                   , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 65         },
	[71 ] = { 198, RO_ATTRS(apnPassword)                   , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 65         },
	[72 ] = { 199, RO_ATTRX(apnControlPoint)               , ATTR_TYPE_U8            , n, y, n, n, y, n, av_cp8              , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[73 ] = { 200, RO_ATTRX(apnStatus)                     , ATTR_TYPE_S32           , n, n, y, n, n, n, av_int32            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[74 ] = { 201, RW_ATTRX(modemDesiredLogLevel)          , ATTR_TYPE_U32           , y, y, y, n, y, n, av_cp32             , NULL                                , .min.ux = 0         , .max.ux = 4          },
	[75 ] = { 202, RO_ATTRX(lteOperatorIndex)              , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[76 ] = { 203, RO_ATTRE(fotaControlPoint)              , ATTR_TYPE_U8            , n, y, n, n, y, n, av_cp8              , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[77 ] = { 204, RO_ATTRE(fotaStatus)                    , ATTR_TYPE_U8            , n, n, y, n, n, n, av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[78 ] = { 205, RO_ATTRS(fotaFileName)                  , ATTR_TYPE_STRING        , n, y, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 64         },
	[79 ] = { 206, RO_ATTRX(fotaSize)                      , ATTR_TYPE_U32           , n, n, y, n, n, n, av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[80 ] = { 207, RO_ATTRX(fotaCount)                     , ATTR_TYPE_U32           , n, n, y, n, n, n, av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[81 ] = { 208, RW_ATTRS(loadPath)                      , ATTR_TYPE_STRING        , y, y, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[82 ] = { 209, RW_ATTRS(dumpPath)                      , ATTR_TYPE_STRING        , y, y, y, n, n, n, av_string           , NULL                                , .min.ux = 0         , .max.ux = 32         },
	[83 ] = { 210, RW_ATTRX(nvImported)                    , ATTR_TYPE_BOOL          , y, n, y, n, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[84 ] = { 211, RW_ATTRX(floaty)                        , ATTR_TYPE_FLOAT         , y, y, y, n, n, n, av_float            , NULL                                , .min.fx = 0.0       , .max.fx = 0.0        },
	[85 ] = { 212, RO_ATTRE(generatePsk)                   , ATTR_TYPE_U8            , n, y, n, n, y, n, av_cp8              , NULL                                , .min.ux = 0         , .max.ux = 1          },
	[86 ] = { 213, RW_ATTRX(lwm2mPsk)                      , ATTR_TYPE_BYTE_ARRAY    , y, n, y, n, n, n, av_array            , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[87 ] = { 214, RW_ATTRS(lwm2mClientId)                 , ATTR_TYPE_STRING        , y, y, y, n, n, n, av_string           , NULL                                , .min.ux = 1         , .max.ux = 32         },
	[88 ] = { 215, RW_ATTRS(lwm2mPeerUrl)                  , ATTR_TYPE_STRING        , y, y, y, n, n, n, av_string           , NULL                                , .min.ux = 6         , .max.ux = 128        },
	[89 ] = { 216, RO_ATTRE(lteInitError)                  , ATTR_TYPE_S8            , n, n, y, n, n, n, av_int8             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[90 ] = { 217, RO_ATTRE(cloudError)                    , ATTR_TYPE_S8            , n, n, y, n, n, n, av_int8             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[91 ] = { 218, RO_ATTRX(commissioningBusy)             , ATTR_TYPE_BOOL          , n, n, y, n, n, n, av_bool             , NULL                                , .min.ux = 0         , .max.ux = 0          },
	[92 ] = { 219, RO_ATTRS(imsi)                          , ATTR_TYPE_STRING        , n, n, y, n, n, n, av_string           , NULL                                , .min.ux = 14        , .max.ux = 15         },
	[93 ] = { 220, RO_ATTRE(modemFunctionality)            , ATTR_TYPE_S8            , n, n, y, n, n, n, av_int8             , attr_prepare_modemFunctionality     , .min.ux = 0         , .max.ux = 0          }
	/* pyend */
};

/**
 * @brief map id to table entry (invalid entries are NULL)
 */
static const struct attr_table_entry * const ATTR_MAP[] = {
	/* pystart - attribute map */
	[1  ] = &ATTR_TABLE[0  ],
	[5  ] = &ATTR_TABLE[1  ],
	[11 ] = &ATTR_TABLE[2  ],
	[12 ] = &ATTR_TABLE[3  ],
	[13 ] = &ATTR_TABLE[4  ],
	[14 ] = &ATTR_TABLE[5  ],
	[16 ] = &ATTR_TABLE[6  ],
	[59 ] = &ATTR_TABLE[7  ],
	[60 ] = &ATTR_TABLE[8  ],
	[61 ] = &ATTR_TABLE[9  ],
	[63 ] = &ATTR_TABLE[10 ],
	[75 ] = &ATTR_TABLE[11 ],
	[93 ] = &ATTR_TABLE[12 ],
	[94 ] = &ATTR_TABLE[13 ],
	[95 ] = &ATTR_TABLE[14 ],
	[140] = &ATTR_TABLE[15 ],
	[142] = &ATTR_TABLE[16 ],
	[143] = &ATTR_TABLE[17 ],
	[144] = &ATTR_TABLE[18 ],
	[145] = &ATTR_TABLE[19 ],
	[146] = &ATTR_TABLE[20 ],
	[147] = &ATTR_TABLE[21 ],
	[148] = &ATTR_TABLE[22 ],
	[149] = &ATTR_TABLE[23 ],
	[150] = &ATTR_TABLE[24 ],
	[151] = &ATTR_TABLE[25 ],
	[152] = &ATTR_TABLE[26 ],
	[153] = &ATTR_TABLE[27 ],
	[154] = &ATTR_TABLE[28 ],
	[155] = &ATTR_TABLE[29 ],
	[157] = &ATTR_TABLE[30 ],
	[158] = &ATTR_TABLE[31 ],
	[159] = &ATTR_TABLE[32 ],
	[160] = &ATTR_TABLE[33 ],
	[161] = &ATTR_TABLE[34 ],
	[162] = &ATTR_TABLE[35 ],
	[163] = &ATTR_TABLE[36 ],
	[164] = &ATTR_TABLE[37 ],
	[165] = &ATTR_TABLE[38 ],
	[166] = &ATTR_TABLE[39 ],
	[167] = &ATTR_TABLE[40 ],
	[168] = &ATTR_TABLE[41 ],
	[169] = &ATTR_TABLE[42 ],
	[170] = &ATTR_TABLE[43 ],
	[171] = &ATTR_TABLE[44 ],
	[172] = &ATTR_TABLE[45 ],
	[173] = &ATTR_TABLE[46 ],
	[174] = &ATTR_TABLE[47 ],
	[175] = &ATTR_TABLE[48 ],
	[176] = &ATTR_TABLE[49 ],
	[177] = &ATTR_TABLE[50 ],
	[178] = &ATTR_TABLE[51 ],
	[179] = &ATTR_TABLE[52 ],
	[180] = &ATTR_TABLE[53 ],
	[181] = &ATTR_TABLE[54 ],
	[182] = &ATTR_TABLE[55 ],
	[183] = &ATTR_TABLE[56 ],
	[184] = &ATTR_TABLE[57 ],
	[185] = &ATTR_TABLE[58 ],
	[186] = &ATTR_TABLE[59 ],
	[187] = &ATTR_TABLE[60 ],
	[188] = &ATTR_TABLE[61 ],
	[189] = &ATTR_TABLE[62 ],
	[190] = &ATTR_TABLE[63 ],
	[191] = &ATTR_TABLE[64 ],
	[192] = &ATTR_TABLE[65 ],
	[193] = &ATTR_TABLE[66 ],
	[194] = &ATTR_TABLE[67 ],
	[195] = &ATTR_TABLE[68 ],
	[196] = &ATTR_TABLE[69 ],
	[197] = &ATTR_TABLE[70 ],
	[198] = &ATTR_TABLE[71 ],
	[199] = &ATTR_TABLE[72 ],
	[200] = &ATTR_TABLE[73 ],
	[201] = &ATTR_TABLE[74 ],
	[202] = &ATTR_TABLE[75 ],
	[203] = &ATTR_TABLE[76 ],
	[204] = &ATTR_TABLE[77 ],
	[205] = &ATTR_TABLE[78 ],
	[206] = &ATTR_TABLE[79 ],
	[207] = &ATTR_TABLE[80 ],
	[208] = &ATTR_TABLE[81 ],
	[209] = &ATTR_TABLE[82 ],
	[210] = &ATTR_TABLE[83 ],
	[211] = &ATTR_TABLE[84 ],
	[212] = &ATTR_TABLE[85 ],
	[213] = &ATTR_TABLE[86 ],
	[214] = &ATTR_TABLE[87 ],
	[215] = &ATTR_TABLE[88 ],
	[216] = &ATTR_TABLE[89 ],
	[217] = &ATTR_TABLE[90 ],
	[218] = &ATTR_TABLE[91 ],
	[219] = &ATTR_TABLE[92 ],
	[220] = &ATTR_TABLE[93 ]
	/* pyend */
};
BUILD_ASSERT(ARRAY_SIZE(ATTR_MAP) == (ATTR_TABLE_MAX_ID + 1),
	     "Invalid attribute map");

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void attr_table_initialize(void)
{
	memcpy(&rw, &DRW, sizeof(rw_attribute_t));
	memcpy(&ro, &DRO, sizeof(ro_attribute_t));
}

void attr_table_factory_reset(void)
{
	size_t i = 0;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		memcpy(ATTR_TABLE[i].pData, ATTR_TABLE[i].pDefault,
		       ATTR_TABLE[i].size);
	}
}

const struct attr_table_entry *const attr_map(attr_id_t id)
{
	if (id > ATTR_TABLE_MAX_ID) {
		return NULL;
	} else {
		return ATTR_MAP[id];
	}
}

attr_index_t attr_table_index(const struct attr_table_entry *const entry)
{
	__ASSERT(PART_OF_ARRAY(ATTR_TABLE, entry), "Invalid entry");
	return (entry - &ATTR_TABLE[0]);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* pystart - prepare for read - weak implementations */
__weak int attr_prepare_upTime(void)
{
	return 0;
}

__weak int attr_prepare_batteryVoltageMv(void)
{
	return 0;
}

__weak int attr_prepare_qrtc(void)
{
	return 0;
}

__weak int attr_prepare_qrtcLastSet(void)
{
	return 0;
}

__weak int attr_prepare_modemBoot(void)
{
	return 0;
}

__weak int attr_prepare_modemFunctionality(void)
{
	return 0;
}

/* pyend */

/* pystart - get string */
const char *const attr_get_string_cert_status(int value)
{
	switch (value) {
		case 1:           return "Busy";
		case 0:           return "Success";
		case -1:          return "Eperm";
		default:          return "?";
	}
}

const char *const attr_get_string_gateway_state(int value)
{
	switch (value) {
		case 0:           return "Power Up Init";
		case 1:           return "Network Init";
		case 2:           return "Wait For Network";
		case 3:           return "Network Connected";
		case 4:           return "Network Disconnected";
		case 5:           return "Network Error";
		case 6:           return "Wait For Commission";
		case 7:           return "Resolve Server";
		case 8:           return "Wait Before Cloud Connect";
		case 9:           return "Cloud Connected";
		case 10:          return "Cloud Wait For Disconnect";
		case 11:          return "Cloud Disconnected";
		case 12:          return "Cloud Error";
		case 13:          return "Fota Busy ";
		case 14:          return "Decommission";
		case 15:          return "Cloud Request Disconnect";
		case 16:          return "Cloud Connecting";
		case 17:          return "Modem Init";
		case 18:          return "Modem Error";
		default:          return "?";
	}
}

const char *const attr_get_string_lte_network_state(int value)
{
	switch (value) {
		case 0:           return "Not Registered";
		case 1:           return "Home Network";
		case 2:           return "Searching";
		case 3:           return "Registration Denied";
		case 4:           return "Out Of Coverage";
		case 5:           return "Roaming";
		case 8:           return "Emergency";
		case 240:         return "Unable To Configure";
		default:          return "?";
	}
}

const char *const attr_get_string_lte_startup_state(int value)
{
	switch (value) {
		case 0:           return "Ready";
		case 1:           return "Waiting For Access Code";
		case 2:           return "Sim Not Present";
		case 3:           return "Sim Lock";
		case 4:           return "Unrecoverable Error";
		case 5:           return "Unknown";
		case 6:           return "Inactive Sim";
		default:          return "?";
	}
}

const char *const attr_get_string_lte_sleep_state(int value)
{
	switch (value) {
		case 0:           return "Uninitialized";
		case 1:           return "Asleep";
		case 2:           return "Awake";
		default:          return "?";
	}
}

const char *const attr_get_string_central_state(int value)
{
	switch (value) {
		case 0:           return "Finding Device";
		case 1:           return "Finding Service";
		case 2:           return "Finding Ess Temperature Char";
		case 3:           return "Finding Ess Humidity Char";
		case 4:           return "Finding Ess Pressure Char";
		case 5:           return "Connected And Configured";
		case 6:           return "Finding Smp Char";
		case 7:           return "Challenge Request";
		case 8:           return "Challenge Response";
		case 9:           return "Log Download";
		default:          return "?";
	}
}

const char *const attr_get_string_modem_boot(int value)
{
	switch (value) {
		case 0:           return "Normal";
		case 1:           return "Delayed";
		case 2:           return "Airplane";
		default:          return "?";
	}
}

const char *const attr_get_string_fota_control_point(int value)
{
	switch (value) {
		case 0:           return "Nop";
		case 2:           return "Modem Start";
		default:          return "?";
	}
}

const char *const attr_get_string_fota_status(int value)
{
	switch (value) {
		case 0:           return "Success";
		case 1:           return "Busy";
		case 2:           return "Error";
		default:          return "?";
	}
}

const char *const attr_get_string_generate_psk(int value)
{
	switch (value) {
		case 0:           return "LwM2M Default";
		case 1:           return "LwM2M Random";
		default:          return "?";
	}
}

const char *const attr_get_string_lte_init_error(int value)
{
	switch (value) {
		case 0:           return "None";
		case -1:          return "No Iface";
		case -2:          return "Iface Cfg";
		case -3:          return "Dns Cfg";
		case -4:          return "Modem";
		case -5:          return "Airplane";
		default:          return "?";
	}
}

const char *const attr_get_string_cloud_error(int value)
{
	switch (value) {
		case 0:           return "None";
		case -1:          return "Init Endpoint";
		case -2:          return "Init Client";
		case -3:          return "Init Root Ca";
		case -4:          return "Read Cred Fs";
		case -5:          return "Cred Size";
		case -6:          return "Init Topic Prefix";
		case -7:          return "Init Client Cert";
		case -8:          return "Init Client Key";
		default:          return "?";
	}
}

const char *const attr_get_string_modem_functionality(int value)
{
	switch (value) {
		case -1:          return "Errno";
		case 0:           return "Minimum";
		case 1:           return "Full";
		case 4:           return "Airplane";
		default:          return "?";
	}
}

/* pyend */