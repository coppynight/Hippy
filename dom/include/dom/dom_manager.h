#pragma once

#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <vector>

#include "base/logging.h"
#include "core/base/task_runner.h"
#include "core/base/common.h"
#include "core/task/common_task.h"
#include "dom/dom_argument.h"
#include "dom/dom_event.h"
#include "dom/dom_listener.h"
#include "dom/dom_value.h"
#include "dom/layout_node.h"

namespace hippy {
inline namespace dom {

class DomNode;
class RenderManager;

class DomManager : public std::enable_shared_from_this<DomManager> {
 public:
  using DomValue = tdf::base::DomValue;
  using TaskRunner = hippy::base::TaskRunner;

  DomManager(uint32_t root_id);
  ~DomManager() = default;

  int32_t GetId() { return id_; }

  inline std::shared_ptr<RenderManager> GetRenderManager() { return render_manager_.lock(); }
  inline void SetRenderManager(std::shared_ptr<RenderManager> render_manager) { render_manager_ = render_manager; }
  inline void SetDelegateTaskRunner(std::shared_ptr<TaskRunner> runner) { delegate_task_runner_ = runner; }
  inline uint32_t GetRootId() { return root_id_; }
  inline std::shared_ptr<DomNode> GetNode(uint32_t id) {
    return dom_node_registry_.GetNode(hippy::base::checked_numeric_cast<uint32_t , int32_t>(id));
  }

  void CreateDomNodes(std::vector<std::shared_ptr<DomNode>>&& nodes);
  void UpdateDomNodes(std::vector<std::shared_ptr<DomNode>>&& nodes);
  void DeleteDomNodes(std::vector<std::shared_ptr<DomNode>>&& nodes);
  void EndBatch();
  // 返回0代表失败，正常id从1开始
  void AddEventListener(uint32_t id, const std::string& name, bool use_capture, const EventCallback& cb,
                        const CallFunctionCallback& callback);
  void RemoveEventListener(uint32_t id, const std::string& name, uint32_t listener_id);
  void CallFunction(uint32_t id, const std::string& name, const DomArgument& param, const CallFunctionCallback& cb);
  std::tuple<float, float> GetRootSize();
  void SetRootSize(float width, float height);
  void SetRootNode(const std::shared_ptr<DomNode>& root_node);
  void DoLayout();
  void PostTask(std::function<void()> func);
  void StartTaskRunner() { dom_task_runner_->Start(); }
  void TerminateTaskRunner() { dom_task_runner_->Terminate(); }
  static void Insert(const std::shared_ptr<DomManager>& dom_manager);
  static std::shared_ptr<DomManager> Find(int32_t id);
  static bool Erase(int32_t id);
  static bool Erase(const std::shared_ptr<DomManager>& dom_manager);

 private:
  int32_t id_;
  uint32_t root_id_;
  std::shared_ptr<DomNode> root_node_;
  std::weak_ptr<RenderManager> render_manager_;
  std::weak_ptr<TaskRunner> delegate_task_runner_;
  std::shared_ptr<TaskRunner> dom_task_runner_;

  class DomNodeRegistry {
   public:
    void AddNode(const std::shared_ptr<DomNode>& node);
    std::shared_ptr<DomNode> GetNode(int32_t id);
    void RemoveNode(int32_t id);

   private:
    std::map<int32_t, std::shared_ptr<DomNode>> nodes_;
  };

  DomNodeRegistry dom_node_registry_;

  using DomOperation = std::function<void(void)>;
  std::vector<DomOperation> batched_operations_;
  std::vector<DomOperation> listener_operations_;
  std::vector<DomOperation> layout_operations_;
  std::vector<std::shared_ptr<DomNode>> layout_changed_nodes_;
  void HandleEvent(const std::shared_ptr<DomEvent>& event);
  void AddLayoutChangedNode(const std::shared_ptr<DomNode>& node);
  void AddEventListenerOperation(const std::shared_ptr<DomNode>& node, const std::string& name);
  void RemoveEventListenerOperation(const std::shared_ptr<DomNode>& node, const std::string& name);
  void DeleteDomNode(const std::shared_ptr<DomNode>& node);
  void UpdateRenderNode(const std::shared_ptr<DomNode>& node);

  friend DomNode;
};

}  // namespace dom
}  // namespace hippy