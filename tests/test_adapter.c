/*
 * test_adapter.c — Device Adapter 注册表与 stub 插件单元测试（M3）
 *
 * 平台：Linux
 *
 * 覆盖：
 *   - registry 注册 / 查找 / 满员
 *   - init_all + poll_all（simulate=true）
 *   - simulate=false 时 stub 返回 0 条
 *
 * 运行：ctest --test-dir build -R test_adapter
 */

#include "ingress/adapter.h"
#include "ingress/stub_adapter.h"
#include "platform/config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* 测试用最小插件：每次 poll 固定返回 1 条 */
typedef struct {
    int init_calls;
} test_plugin_ctx_t;

static test_plugin_ctx_t g_test_ctx;

static int test_init(device_adapter_t *self, const gateway_config_t *cfg)
{
    test_plugin_ctx_t *ctx = (test_plugin_ctx_t *)self->ctx;
    (void)cfg;
    if (ctx != NULL) {
        ctx->init_calls++;
    }
    return 0;
}

static void test_shutdown(device_adapter_t *self)
{
    (void)self;
}

static int test_poll(device_adapter_t *self, telemetry_t *out, size_t max_out)
{
    (void)self;
    if (out == NULL || max_out == 0U) {
        return -1;
    }
    telemetry_init(out, 1U, "test_dev", "test_point", 42.0, "U", 99U);
    return 1;
}

static int test_write(device_adapter_t *self, const command_t *cmd)
{
    (void)self;
    (void)cmd;
    return -1;
}

static void fill_test_plugin(device_adapter_t *out)
{
    memset(out, 0, sizeof(*out));
    snprintf(out->name, sizeof(out->name), "test");
    out->protocol = DEVICE_PROTOCOL_TCP;
    out->source_id = 99U;
    out->ctx = &g_test_ctx;
    out->init = test_init;
    out->shutdown = test_shutdown;
    out->poll = test_poll;
    out->write_command = test_write;
}

int main(void)
{
    adapter_registry_t reg;
    gateway_config_t cfg;
    device_adapter_t stub;
    device_adapter_t test_plugin;
    telemetry_t out[8];
    int n;

    memset(&g_test_ctx, 0, sizeof(g_test_ctx));

    /* --- 注册与查找 --- */
    adapter_registry_init(&reg);
    fill_test_plugin(&test_plugin);
    assert(adapter_registry_register(&reg, &test_plugin) == ADAPTER_OK);
    assert(adapter_registry_find(&reg, DEVICE_PROTOCOL_TCP) != NULL);
    assert(adapter_registry_find(&reg, DEVICE_PROTOCOL_MODBUS_RTU) == NULL);

    /* --- init + poll 测试插件 --- */
    gateway_config_defaults(&cfg);
    assert(adapter_registry_init_all(&reg, &cfg) == 0);
    assert(g_test_ctx.init_calls == 1);
    n = adapter_registry_poll_all(&reg, out, 8U);
    assert(n == 1);
    assert(strcmp(out[0].point_id, "test_point") == 0);
    adapter_registry_shutdown_all(&reg);

    /* --- stub：simulate=true 返回 3 条 --- */
    adapter_registry_init(&reg);
    stub_adapter_fill(&stub);
    assert(adapter_registry_register(&reg, &stub) == ADAPTER_OK);
    assert(gateway_config_load("configs/gateway.json", &cfg) == 0);
    assert(cfg.simulate == true);
    assert(adapter_registry_init_all(&reg, &cfg) == 0);
    n = adapter_registry_poll_all(&reg, out, 8U);
    assert(n == 3);
    assert(strcmp(out[0].point_id, "soc_percent") == 0);
    assert(strcmp(out[2].point_id, "grid_power_kw") == 0);
    adapter_registry_shutdown_all(&reg);

    /* --- stub：simulate=false 返回 0 条 --- */
    adapter_registry_init(&reg);
    stub_adapter_fill(&stub);
    assert(adapter_registry_register(&reg, &stub) == ADAPTER_OK);
    cfg.simulate = false;
    assert(adapter_registry_init_all(&reg, &cfg) == 0);
    n = adapter_registry_poll_all(&reg, out, 8U);
    assert(n == 0);
    adapter_registry_shutdown_all(&reg);

    printf("test_adapter passed\n");
    return 0;
}
