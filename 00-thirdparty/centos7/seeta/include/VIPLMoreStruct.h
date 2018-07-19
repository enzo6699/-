#ifndef __VIPL_STRUCT_H__
#define __VIPL_STRUCT_H__


#include "VIPLStruct.h"

/**
 * \brief 跟踪结果
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

	int PID;			/**< 人员编号，相同跟踪人员是唯一的，从 0 开始增长 */
	int frame_no;		/**< 该人员对应的帧号 */
	int step;			/**< step 区分检测状态，0 为直接检测结果，大于 1 为跟踪预测结果，建议去除 step 不为 0 的结果 */

	VIPLFaceInfo pos;	/**< 人脸的位置 */
};

#endif