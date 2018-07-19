#ifndef VIPL_C_STRUCT_H
#define VIPL_C_STRUCT_H

#include "VIPLStruct.h"

#define VIPL_C_API extern "C" VIPL_API

/**
* \brief ģ�������豸
*/
enum VIPLCDevice
{
	VIPL_C_DECIVE_AUTO,	/**< �Զ���⣬������ʹ�� GPU */
	VIPL_C_DECIVE_CPU,	/**< ʹ�� CPU ���� */
	VIPL_C_DECIVE_GPU,	/**< ʹ�� GPU ���㣬�ȼ���GPU0 */
	VIPL_C_DECIVE_GPU0,   /**< Ԥ����GPU��ţ�0�ſ� */
	VIPL_C_DECIVE_GPU1,
	VIPL_C_DECIVE_GPU2,
	VIPL_C_DECIVE_GPU3,
	VIPL_C_DECIVE_GPU4,
	VIPL_C_DECIVE_GPU5,
	VIPL_C_DECIVE_GPU6,
	VIPL_C_DECIVE_GPU7,
};

#endif // VIPL_C_STRUCT_H
