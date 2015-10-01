/// HEADER
#include <csapex/manager/message_renderer_manager.h>

/// COMPONENT
#include <csapex/plugin/plugin_manager.hpp>
#include <csapex/core/settings.h>
#include <csapex/msg/apex_message_provider.h>

/// SYSTEM
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;
using namespace csapex;

MessageRendererManager::MessageRendererManager()
    : manager_(new PluginManager<MessageRenderer>("csapex::MessageRenderer"))
{
}

MessageRendererManager::~MessageRendererManager()
{
}

void MessageRendererManager::setPluginLocator(PluginLocatorPtr locator)
{
    plugin_locator_ = locator;
}

void MessageRendererManager::shutdown()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    renderers.clear();
    manager_.reset();
}

void MessageRendererManager::loadPlugins()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if(!manager_->pluginsLoaded()) {
        manager_->load(plugin_locator_.get());
    }

    renderers.clear();

    typedef std::pair<std::string, PluginManager<csapex::MessageRenderer>::Constructor> PAIR;
    for(PAIR pair : manager_->availableClasses()) {
        try {
            MessageRenderer::Ptr renderer(pair.second());
            renderers[renderer->messageType()] = renderer;

        } catch(const std::exception& e) {
            std::cerr << "cannot load message renderer " << pair.first << ": " << typeid(e).name() << ", what=" << e.what() << std::endl;
        }
    }
}

MessageRendererPtr MessageRendererManager::createMessageRenderer(const ConnectionTypeConstPtr& message)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    if(!manager_->pluginsLoaded() || renderers.empty()) {
        loadPlugins();
    }
    if(renderers.empty()) {
        throw std::runtime_error("no message renderers registered!");
    }

    try {
        return renderers.at(std::type_index(typeid(*message)));
    } catch(const std::exception& e) {
        throw std::runtime_error(std::string("cannot create message renderer for ") + type2name(typeid(*message)));
    }
}
