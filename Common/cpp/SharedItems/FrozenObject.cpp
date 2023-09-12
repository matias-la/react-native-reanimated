#include "FrozenObject.h"
#include "RuntimeManager.h"
#include "ShareableValue.h"
#include "SharedParent.h"

namespace reanimated {

jsi::PropNameID workletCodePropName(jsi::Runtime &rt) {
  jsi::Function symbolFor = rt.global()
    .getPropertyAsObject(rt, "Symbol")
    .getPropertyAsFunction(rt, "for");
  jsi::Value val = symbolFor.call(rt, jsi::String::createFromAscii(rt, "__reanimated_workletCode"));
  jsi::Symbol sym = val.asSymbol(rt);
  return jsi::PropNameID::forSymbol(rt, sym);
}

FrozenObject::FrozenObject(
    jsi::Runtime &rt,
    const jsi::Object &object,
    RuntimeManager *runtimeManager) {
  auto propertyNames = object.getPropertyNames(rt);
  const size_t count = propertyNames.size(rt);
  namesOrder.reserve(count);
  for (size_t i = 0; i < count; i++) {
    auto propertyName = propertyNames.getValueAtIndex(rt, i).asString(rt);
    namesOrder.push_back(propertyName.utf8(rt));
    std::string nameStr = propertyName.utf8(rt);
    if (nameStr == "__reanimated_workletCode")
      continue;
    map[nameStr] = ShareableValue::adapt(
        rt, object.getProperty(rt, propertyName), runtimeManager);
    this->containsHostFunction |= map[nameStr]->containsHostFunction;
  }
  jsi::PropNameID prop = workletCodePropName(rt);
  if (object.hasProperty(rt, prop)) {
    namesOrder.push_back("__reanimated_workletCode");
    map["__reanimated_workletCode"] = ShareableValue::adapt(
	rt, object.getProperty(rt, prop), runtimeManager);
    this->containsHostFunction |= map["__reanimated_workletCode"]->containsHostFunction;
  }
}

jsi::Object FrozenObject::shallowClone(jsi::Runtime &rt) {
  jsi::Object object(rt);
  for (auto propName : namesOrder) {
    auto value = map[propName];
    object.setProperty(
        rt, jsi::String::createFromUtf8(rt, propName), value->getValue(rt));
  }
  return object;
}

} // namespace reanimated
