/*
 * test_config.c — Config Manager 单元测试（M2）
 *
 * 平台：Linux（在项目根目录运行，依赖相对路径 configs/gateway.json）
 *
 * 覆盖范围：
 *   - gateway_config_defaults：内置默认值
 *   - gateway_config_load：从示例 JSON 加载并覆盖
 *   - 缺失文件时返回失败
 *
 * 运行：
 *   ctest --test-dir build -R test_config
 *   或 ./build/test_config  （工作目录须为仓库根）
 */

#include "platform/config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    gateway_config_t cfg;

    /* --- 默认值：不读文件，验证内置回退 --- */
    gateway_config_defaults(&cfg);
    assert(strcmp(cfg.device_id, "gw-dev") == 0);
    assert(cfg.simulate == true);
    assert(cfg.poll_interval_ms == 1000U);
    assert(cfg.broker_port == 1883U);
    assert(strcmp(cfg.serial_device, "/dev/ttyUSB0") == 0);
    assert(strcmp(cfg.log_dir, "/tmp/edgeflow") == 0);

    /* --- 从仓库 configs/gateway.json 加载 --- */
    assert(gateway_config_load("configs/gateway.json", &cfg) == 0);
    assert(strcmp(cfg.device_id, "gw-001") == 0);
    assert(cfg.simulate == true);
    assert(cfg.poll_interval_ms == 500U);
    assert(cfg.queue_size == 4096U);
    assert(cfg.temp_high == 60.0);
    assert(strcmp(cfg.broker_host, "127.0.0.1") == 0);
    assert(strcmp(cfg.log_dir, "/tmp/edgeflow") == 0);

    /* --- 错误路径：文件不存在必须失败 --- */
    assert(gateway_config_load("configs/no_such_file.json", &cfg) != 0);

    printf("test_config passed\n");
    return 0;
}
