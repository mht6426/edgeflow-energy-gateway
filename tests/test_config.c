#include "platform/config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    gateway_config_t cfg;

    gateway_config_defaults(&cfg);
    assert(cfg.simulate);
    assert(cfg.poll_interval_ms == 500U);
    assert(cfg.peak_shaving_threshold_kw == 120.0);
    assert(cfg.max_discharge_kw == 50.0);
    assert(cfg.min_discharge_soc == 30.0);

    assert(gateway_config_load("configs/gateway.json", &cfg) == 0);
    assert(strcmp(cfg.device_id, "gw-001") == 0);
    assert(cfg.peak_shaving_threshold_kw == 120.0);

    printf("test_config passed\n");
    return 0;
}
