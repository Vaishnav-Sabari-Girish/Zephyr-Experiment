/*
 * GY-91 Pitch / Yaw / Roll  —  NXP FRDM-MCXA156  (Zephyr RTOS)
 *
 * Mimics the Arduino sketch output format:
 *   Pitch: <val>, Yaw: <val>, Roll: <val>
 *
 * Fuses MPU9250 9-axis IMU data with a Mahony AHRS filter.
 * Sample rate: 100 Hz   Print rate: ~20 Hz (every 50 ms)
 *
 * Wiring: GY-91 SCL→P3_27  SDA→P3_28  VCC→3V3  GND→GND
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <math.h>
#include <stdio.h>

/* ── MPU9250 device ─────────────────────────────────────────────── */
#define MPU9250_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(invensense_mpu9250)
static const struct device *const mpu = DEVICE_DT_GET(MPU9250_NODE);

/* ══════════════════════════════════════════════════════════════════
 *  MAHONY  AHRS   (9‑axis  quaternion‑based  complementary  filter)
 * ══════════════════════════════════════════════════════════════════ */

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define MAHONY_KP     0.5f
#define MAHONY_KI     0.005f
#define MAHONY_DT_MIN 0.001f

static float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };  /* w, x, y, z */
static float integral_fb[3];

static void mahony_update(float gx, float gy, float gz,  /* rad/s */
                          float ax, float ay, float az,  /* m/s²  */
                          float mx, float my, float mz,  /* µT    */
                          float dt)
{
	float recip_norm;
	float half_vx, half_vy, half_vz;
	float half_ex, half_ey, half_ez;
	float qa, qb, qc;
	float hx, hy, bx, bz;

	if (dt < MAHONY_DT_MIN) dt = MAHONY_DT_MIN;

	/* normalise accel — guard against zero-length (sensor not ready) */
	{
		float len = sqrtf(ax * ax + ay * ay + az * az);
		if (len < 1e-6f) len = 1e-6f;
		recip_norm = 1.0f / len;
		ax *= recip_norm;
		ay *= recip_norm;
		az *= recip_norm;
	}

	/* normalise magnetometer — guard against zero-length */
	{
		float len = sqrtf(mx * mx + my * my + mz * mz);
		if (len < 1e-6f) len = 1e-6f;
		recip_norm = 1.0f / len;
		mx *= recip_norm;
		my *= recip_norm;
		mz *= recip_norm;
	}

	/* estimated gravity direction from quaternion */
	half_vx = q[1] * q[3] - q[0] * q[2];
	half_vy = q[0] * q[1] + q[2] * q[3];
	half_vz = q[0] * q[0] - 0.5f + q[3] * q[3];

	/* earth-frame flux */
	hx = 2.0f * (mx * (0.5f - q[2] * q[2] - q[3] * q[3])
		   + my * (q[1] * q[2] - q[0] * q[3])
		   + mz * (q[1] * q[3] + q[0] * q[2]));
	hy = 2.0f * (mx * (q[1] * q[2] + q[0] * q[3])
		   + my * (0.5f - q[1] * q[1] - q[3] * q[3])
		   + mz * (q[2] * q[3] - q[0] * q[1]));
	bx = sqrtf(hx * hx + hy * hy);
	bz = 2.0f * (mx * (q[1] * q[3] - q[0] * q[2])
		   + my * (q[2] * q[3] + q[0] * q[1])
		   + mz * (0.5f - q[1] * q[1] - q[2] * q[2]));

	/* body-frame flux reference */
	half_vx += bx * (0.5f - q[2] * q[2] - q[3] * q[3])
		 + bz * (q[1] * q[3] - q[0] * q[2]);
	half_vy += bx * (q[1] * q[2] - q[0] * q[3])
		 + bz * (q[0] * q[1] + q[2] * q[3]);
	half_vz += bx * (q[0] * q[2] + q[1] * q[3])
		 + bz * (0.5f - q[1] * q[1] - q[2] * q[2]);

	/* error = cross(measured, estimated) */
	half_ex = (ay * half_vz - az * half_vy)
		+ (my * half_vz - mz * half_vy);
	half_ey = (az * half_vx - ax * half_vz)
		+ (mz * half_vx - mx * half_vz);
	half_ez = (ax * half_vy - ay * half_vx)
		+ (mx * half_vy - my * half_vx);

	/* integral feedback */
	if (MAHONY_KI > 0.0f) {
		integral_fb[0] += MAHONY_KI * half_ex * dt;
		integral_fb[1] += MAHONY_KI * half_ey * dt;
		integral_fb[2] += MAHONY_KI * half_ez * dt;
		gx += integral_fb[0];
		gy += integral_fb[1];
		gz += integral_fb[2];
	}

	/* proportional feedback */
	gx += MAHONY_KP * half_ex;
	gy += MAHONY_KP * half_ey;
	gz += MAHONY_KP * half_ez;

	/* integrate quaternion */
	gx *= 0.5f * dt;
	gy *= 0.5f * dt;
	gz *= 0.5f * dt;

	qa = q[0];  qb = q[1];  qc = q[2];

	q[0] += -qb * gx - qc * gy - q[3] * gz;
	q[1] +=  qa * gx + qc * gz - q[3] * gy;
	q[2] +=  qa * gy - qb * gz + q[3] * gx;
	q[3] +=  qa * gz + qb * gy - qc * gx;

	/* re-normalise */
	recip_norm = 1.0f / sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
	q[0] *= recip_norm;
	q[1] *= recip_norm;
	q[2] *= recip_norm;
	q[3] *= recip_norm;
}

static void quat_to_euler(float w, float x, float y, float z,
                          float *yaw, float *pitch, float *roll)
{
	float sinr_cosp = 2.0f * (w * x + y * z);
	float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
	float sinp       = 2.0f * (w * y - z * x);
	float siny_cosp  = 2.0f * (w * z + x * y);
	float cosy_cosp  = 1.0f - 2.0f * (y * y + z * z);

	*roll  = atan2f(sinr_cosp, cosr_cosp) * 180.0f / (float)M_PI;
	*pitch = asinf(sinp)                  * 180.0f / (float)M_PI;
	*yaw   = atan2f(siny_cosp, cosy_cosp) * 180.0f / (float)M_PI;
}

/* ═════════════════════════════════════════════════════════════════ */

int main(void)
{
	int64_t last_ts;

	if (mpu == NULL) {
		printk("Error: MPU9250 not found in devicetree\n");
		return -1;
	}
	if (!device_is_ready(mpu)) {
		printk("Error: MPU9250 device not ready\n");
		return -1;
	}

	printk("GY-91 Pitch/Yaw/Roll ready\n");

	last_ts = k_uptime_ticks();

	while (1) {
		struct sensor_value accel[3], gyro[3], magn[3];
		float ax, ay, az, gx, gy, gz, mx, my, mz;
		float yaw, pitch, roll;
		float dt;
		int64_t now;

		/* ── read MPU9250 ── */
		if (sensor_sample_fetch(mpu) < 0) {
			k_msleep(5);
			continue;
		}

		sensor_channel_get(mpu, SENSOR_CHAN_ACCEL_XYZ, accel);
		sensor_channel_get(mpu, SENSOR_CHAN_GYRO_XYZ,  gyro);
		sensor_channel_get(mpu, SENSOR_CHAN_MAGN_XYZ,  magn);

		ax = sensor_value_to_double(&accel[0]);
		ay = sensor_value_to_double(&accel[1]);
		az = sensor_value_to_double(&accel[2]);

		gx = sensor_value_to_double(&gyro[0]);
		gy = sensor_value_to_double(&gyro[1]);
		gz = sensor_value_to_double(&gyro[2]);

		mx = sensor_value_to_double(&magn[0]);
		my = sensor_value_to_double(&magn[1]);
		mz = sensor_value_to_double(&magn[2]);

		/* print raw readings for first 2 seconds to diagnose */
		static int dbg_cnt;
		if (dbg_cnt < 10) {
			dbg_cnt++;
			printk("raw acc:%.2f,%.2f,%.2f gyr:%.2f,%.2f,%.2f mag:%.1f,%.1f,%.1f\n",
			       (double)ax, (double)ay, (double)az,
			       (double)gx, (double)gy, (double)gz,
			       (double)mx, (double)my, (double)mz);
		}

		/* ── dt ── */
		now = k_uptime_ticks();
		dt = (float)(now - last_ts)
		   / (float)CONFIG_SYS_CLOCK_TICKS_PER_SEC;
		last_ts = now;

		/* ── filter ── */
		mahony_update(gx, gy, gz, ax, ay, az, mx, my, mz, dt);
		quat_to_euler(q[0], q[1], q[2], q[3],
			      &yaw, &pitch, &roll);

		/* ── print ~20 Hz, Arduino‑style ── */
		static int cnt;
		if (++cnt >= 5) {
			cnt = 0;
			printk("Pitch: %.2f, Yaw: %.2f, Roll: %.2f\n",
			       (double)pitch, (double)yaw, (double)roll);
		}

		k_msleep(10);   /* 100 Hz loop */
	}

	return 0;
}
