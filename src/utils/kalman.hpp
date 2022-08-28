#ifndef SCALAR_KALMAN_H
#define SCALAR_KALMAN_H

/**
 * 	x = A*x + w; w~N(0,Q)
 * 	y = C*x + v; v~N(0,R)
 */
typedef struct scalar_kalman_s
{
	float A, C;
	float A2, C2; 	// A,C 的平方

	float Q, R;
	float K, P;

	float x;
}scalar_kalman_t;

/**
* 	状态方程: x = A*x + w; w~N(0,Q)
* 	测量方程: y = C*x + v; v~N(0,R)
 */

template<typename T>
inline float scalar_kalman(T kalman, float y){
    // 状态预测
    kalman->x = kalman->A * kalman->x;
    // 误差协方差预测
    kalman->P = kalman->A2 * kalman->P + kalman->Q;
    // 计算卡尔曼滤波增益
    kalman->K = kalman->P * kalman->C / (kalman->C2 * kalman->P + kalman->R);

    // 状态估计校正
    kalman->x = kalman->x + kalman->K * (y - kalman->C * kalman->x);
    // 误差协方差估计校正
    kalman->P = (1 - kalman->K * kalman->C) * kalman->P;

    return kalman->C * kalman->x; 	// 输出滤波后的y
}

template<typename T>
inline void scalar_kalman_init(T kalman, float A, float C, float Q, float R){
    kalman->A = A;
    kalman->A2 = A * A;
    kalman->C = C;
    kalman->C2 = C * C;

    kalman->Q = Q;
    kalman->R = R;

    kalman->x = 0;
    kalman->P = Q;
    kalman->K = 1;
}

#endif
