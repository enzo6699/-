#ifndef __VIPL_STRUCT_H__
#define __VIPL_STRUCT_H__


#include "VIPLStruct.h"

/**
 * \brief ���ٽ��
 */
struct VIPLTrackingInfo
{
	VIPLTrackingInfo(int _pid, int _frame_no, int _x, int _y, int _width, int _height, int _step = 0){
		PID = _pid;
		frame_no = _frame_no;
		step = _step;
		pos.x = _x;
		pos.y = _y;
		pos.width = _width;
		pos.height = _height;
	};
	VIPLTrackingInfo(int _pid, int _frame_no, VIPLFaceInfo& _pos, int _step = 0){
		PID = _pid;
		frame_no = _frame_no;
		step = _step;
		pos = _pos;
	};

	int PID;			/**< ��Ա��ţ���ͬ������Ա��Ψһ�ģ��� 0 ��ʼ���� */
	int frame_no;		/**< ����Ա��Ӧ��֡�� */
	int step;			/**< step ���ּ��״̬��0 Ϊֱ�Ӽ���������� 1 Ϊ����Ԥ����������ȥ�� step ��Ϊ 0 �Ľ�� */

	VIPLFaceInfo pos;	/**< ������λ�� */
};

#endif