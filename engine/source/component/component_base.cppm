module;

#include <cstdint>

export module zeytin.component;
import zeytin.entity;

/// Base class for components
export struct Component 
{
  virtual ~Component() = default;

  /// Prefer using this over the constructor,
  /// because serializer will read and set properties after the component
  /// constructored.
  virtual void on_init() {}

  /// Called upon entering the play mode
  virtual void on_play_start() {}

  /// Called every frame
  virtual void on_early_update() {}

  /// Called every frame
  virtual void on_update() {}

  /// Called every frame while in play mode
  virtual void on_play_update() {}

  /// Called every frame while in play mode, right after on_play_update
  virtual void on_play_late_update() {}

  [[nodiscard]]
  uint64_t get_id() const { return entity_id; }

  EntityID entity_id = 0;
  bool is_dead = false;
  bool post_inited = false;
};

/// Defines empty lifetime functions. Can be used for data-only components
export struct DataComponent : Component
{
  virtual ~DataComponent() = default;

  void on_init() final  {}
  void on_play_start() final {}
  void on_early_update() final {}
  void on_update() final {}
  void on_play_update() final {}
  void on_play_late_update() final {}
};
