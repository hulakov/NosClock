import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Define the C++ namespace and class
nosclock_ns = cg.esphome_ns.namespace("nosclock")
NosController = nosclock_ns.class_("NosController", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(NosController),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add_library("FastLED", None)
