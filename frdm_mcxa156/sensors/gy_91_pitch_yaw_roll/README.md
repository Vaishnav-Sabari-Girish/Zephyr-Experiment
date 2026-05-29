# GY-91 Pitch / Yaw / Roll — FRDM-MCXA156

Fuses MPU9250 9-axis IMU (accel + gyro + magnetometer) into Euler angles using a
Mahony AHRS complementary filter.

[Code File](./src/main.c)

## Output format (matches Arduino sketch)

```text
raw acc:0.12,-0.08,9.81 gyr:0.01,0.00,-0.02 mag:23.1,-5.4,40.2
Pitch: -2.35, Yaw: 12.40, Roll: 1.08
Pitch: -2.33, Yaw: 12.38, Roll: 1.06
```

Raw sensor values print for the first ~1 second, then the filter output runs at
~20 Hz.

---

## Diff from the raw-sensor program

The previous program printed 10 raw sensor channels every second using
`LOG_INF`. This version **replaces all of that** with a sensor-fusion pipeline.

### What was removed

| Removed | Why |
|---------|-----|
| `#include <zephyr/logging/log.h>` + `LOG_MODULE_REGISTER` | Replaced with `printk` to match Arduino `Serial.print` style |
| `BMP280_NODE`, `bmp`, `bmp280_read()` | BMP280 not needed — pitch/yaw/roll come from IMU only |
| `sensor_check()` helper | Inlined — only one device to check now |
| `mpu9250_read()` function | Split into inline sensor reads + filter pipeline |
| 1 Hz loop with `k_msleep(1000)` | Must run at **100 Hz** for gyro integration to work |

### What was added

```diff
-#include <zephyr/logging/log.h>
+#include <math.h>
+#include <stdio.h>

-LOG_MODULE_REGISTER(gy91_logger, LOG_LEVEL_DBG);
+/* ── M_PI not defined by picolibc ── */
+#ifndef M_PI
+#define M_PI 3.14159265358979323846f
+#endif
```

**Filter constants:**

```diff
+/* Mahony AHRS tunables */
+#define MAHONY_KP     0.5f    /* proportional gain — higher = faster convergence */
+#define MAHONY_KI     0.005f  /* integral gain — cancels gyro bias drift */
+#define MAHONY_DT_MIN 0.001f  /* prevents div-by-zero on first sample */
+
+static float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };  /* quaternion (w,x,y,z) */
+static float integral_fb[3];                        /* gyro bias estimate */
```

**Mahony AHRS filter** (~100 lines):

```diff
+static void mahony_update(float gx, float gy, float gz,  /* rad/s */
+                          float ax, float ay, float az,  /* m/s²  */
+                          float mx, float my, float mz,  /* µT    */
+                          float dt)
+{
+    /* 1. Normalise accel & mag (with zero-length guard) */
+    /* 2. Compute gravity & flux direction from quaternion */
+    /* 3. Cross-product error = measured ⊗ estimated */
+    /* 4. PI feedback onto gyro rates */
+    /* 5. Integrate quaternion (q̇ = ½ q ⊗ ω) */
+    /* 6. Re-normalise */
+}
```

**Quaternion → Euler conversion:**

```diff
+static void quat_to_euler(float w, float x, float y, float z,
+                          float *yaw, float *pitch, float *roll)
+{
+    /* Tait-Bryan ZYX convention, output in degrees */
+    *roll  = atan2f(2(wx+yz), 1-2(x²+y²)) · 180/π;
+    *pitch = asinf(2(wy-zx))              · 180/π;
+    *yaw   = atan2f(2(wz+xy), 1-2(y²+z²)) · 180/π;
+}
```

**Main loop — 100 Hz with dt tracking:**

```diff
-while (1) {
-    mpu9250_read();
-    bmp280_read();
-    k_msleep(1000);
-}

+int64_t last_ts = k_uptime_ticks();
+
+while (1) {
+    /* read all 9 axes */
+    sensor_sample_fetch(mpu);
+    sensor_channel_get(mpu, SENSOR_CHAN_ACCEL_XYZ, accel);
+    sensor_channel_get(mpu, SENSOR_CHAN_GYRO_XYZ,  gyro);
+    sensor_channel_get(mpu, SENSOR_CHAN_MAGN_XYZ,  magn);
+
+    /* compute dt from tick delta */
+    int64_t now = k_uptime_ticks();
+    float dt = (now - last_ts) / (float)CONFIG_SYS_CLOCK_TICKS_PER_SEC;
+    last_ts = now;
+
+    /* run filter */
+    mahony_update(gx, gy, gz, ax, ay, az, mx, my, mz, dt);
+    quat_to_euler(q[0], q[1], q[2], q[3], &yaw, &pitch, &roll);
+
+    /* print at ~20 Hz (every 5th loop at 100 Hz) */
+    static int cnt;
+    if (++cnt >= 5) {
+        cnt = 0;
+        printk("Pitch: %.2f, Yaw: %.2f, Roll: %.2f\n",
+               (double)pitch, (double)yaw, (double)roll);
+    }
+
+    k_msleep(10);
+}
```

**Zero-length guard** (stops NaN when sensor returns all zeros on startup):

```diff
-/* normalise accel */
-recip_norm = 1.0f / sqrtf(ax*ax + ay*ay + az*az);

+/* normalise accel — guard against zero-length */
+{
+    float len = sqrtf(ax*ax + ay*ay + az*az);
+    if (len < 1e-6f) len = 1e-6f;
+    recip_norm = 1.0f / len;
+}
```

**Startup diagnostic** (prints raw readings for ~1 second):

```diff
+static int dbg_cnt;
+if (dbg_cnt < 10) {
+    dbg_cnt++;
+    printk("raw acc:%.2f,%.2f,%.2f gyr:%.2f,%.2f,%.2f mag:%.1f,%.1f,%.1f\n", ...);
+}
```

---

## Data flow

```text
┌──────────┐   100 Hz    ┌───────────────┐   20 Hz     ┌──────────┐
│ MPU9250  │───────────►│ Mahony AHRS   │───────────►│ printk() │
│ 9 axes   │  raw float │ q → Euler     │  deg °     │ serial   │
└──────────┘            └──────┬────────┘            └──────────┘
                               │
                     ┌─────────┴─────────┐
                     │ Kp·error (fast)   │
                     │ Ki·∫error (drift) │
                     └───────────────────┘
```

## Tuning

| Symptom | Fix |
|---------|-----|
| Angles wobble | Lower `MAHONY_KP` to 0.1–0.3 |
| Yaw drifts after motion | Raise `MAHONY_KI` to 0.01–0.02 |
| Pitch/roll overshoots | Lower `MAHONY_KP`, check loop rate ≥100 Hz |
| Yaw jumps when tilting | Magnetometer needs hard/soft iron calibration |

## Wiring

| GY-91 | FRDM-MCXA156 |
|-------|--------------|
| VCC   | 3.3V         |
| GND   | GND          |
| SCL   | P3_27        |
| SDA   | P3_28        |

## Build & Flash

```bash
just pristine
just flash
# or:
west build -b frdm_mcxa156 -p always
west flash --runner jlink
```

Serial console: 115200 8N1
