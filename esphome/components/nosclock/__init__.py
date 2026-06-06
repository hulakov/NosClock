import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

nosclock_ns = cg.esphome_ns.namespace('nos_clock')
NosClock = nosclock_ns.class_('NosClock', cg.Component)

cg.add_library("FastLED", None)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(NosClock),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
