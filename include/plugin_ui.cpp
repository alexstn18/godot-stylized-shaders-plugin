#include "plugin_ui.hpp"
#include <godot_cpp/classes/resource_preloader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

void PluginUI::_bind_methods()
{
}

PluginUI::PluginUI()
{
}

PluginUI::~PluginUI()
{
    if(m_panel) m_panel->queue_free();
}

void PluginUI::_enter_tree()
{
    UtilityFunctions::print("Loading scene...");
    Ref<PackedScene> ui_scene = ResourceLoader::get_singleton()->load("res://addons/GodotStylizedShadersPlugin/scenes/plugin_panel.tscn");
    
    if(!ui_scene.is_valid())
    {
        UtilityFunctions::push_error("UI scene is invalid or not found");
        return;
    }
    
    UtilityFunctions::print("Scene loaded, instantiating...");
    Node *instance = ui_scene->instantiate();

    if(!instance)
    {
        UtilityFunctions::push_error("Failed to instantiate scene");
        return;
    }
    
    UtilityFunctions::print("Scene instantiated, casting...");
    m_panel = Object::cast_to<ToolPanel>(instance);
    
    if(!m_panel)
    {
        UtilityFunctions::push_error("Failed to cast to ToolPanel");
        instance->queue_free();
        return;
    }
    
    UtilityFunctions::print("Adding to dock...");
    add_control_to_dock(EditorPlugin::DOCK_SLOT_LEFT_BL, m_panel);
    UtilityFunctions::print("Successfully added to dock");
}

void PluginUI::_exit_tree()
{
    if(m_panel)
    {
        remove_control_from_docks(m_panel);
        m_panel->queue_free();
    }
}
