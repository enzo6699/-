#ifndef VIPL_C_STRUCT_H
#define VIPL_C_STRUCT_H

#include "VIPLStruct.h"

#define VIPL_C_API extern "C" VIPL_API

/**
* \brief 模型运行设备
*/
enum VIPLCDevice
{
	VIPL_C_DECIVE_AUTO,	/**< 自动检测，会优先使用 GPU */
	VIPL_C_DECIVE_CPU,	/**< 使用 CPU 计算 */
	VIPL_C_DECIVE_GPU,	/**< 使用 GPU 计算，等价于GPU0 */
	VIPL_C_DECIVE_GPU0,   /**< 预定义GPU编号，0号卡 */
	VIPL_C_DECIVE_GPU1,
	VIPL_C_DECIVE_GPU2,
	VIPL_C_DECIVE_GPU3,
	VIPL_C_DECIVE_GPU4,
	VIPL_C_DECIVE_GPU5,
	VIPL_C_DECIVE_GPU6,
	VIPL_C_DECIVE_GPU7,
};

#endif // VIPL_C_STRUCT_H
