#pragma once
#ifndef SIMPLE_JSON_PLUGIN_H
#define SIMPLE_JSON_PLUGIN_H

#include "../../include/StreamingInterface.h"

namespace SimpleJsonPlugin {

class SimpleJsonPluginFactory final : public Firebird::IStreamPluginFactoryImpl<SimpleJsonPluginFactory, Firebird::ThrowStatusWrapper> {
public:
    SimpleJsonPluginFactory() = delete;
    explicit SimpleJsonPluginFactory(Firebird::IMaster* master);

    Firebird::IStreamPlugin* createPlugin(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config,
        Firebird::IStringEncodeUtils* encodeUtils, Firebird::IStreamLogger* logger) override;

private:
    Firebird::IMaster* m_master = nullptr;
};

} // namespace SimpleJsonPlugin

#endif // SIMPLE_JSON_PLUGIN_H
