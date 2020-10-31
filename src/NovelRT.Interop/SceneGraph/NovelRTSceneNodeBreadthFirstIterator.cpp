// Copyright © Matt Jones and Contributors. Licensed under the MIT Licence (MIT). See LICENCE.md in the repository root for more information.

#include <stdint.h>
#include <list>
#include "../NovelRTInteropErrorHandlingInternal.h"
#include "NovelRT.Interop/SceneGraph/NovelRTSceneNodeBreadthFirstIterator.h"
#include "NovelRT.Interop/SceneGraph/NovelRTSceneNode.h"
#include "NovelRT.Interop/NovelRTInteropUtils.h"
#include "NovelRT.h"
using namespace NovelRT;

int32_t(*_function)(NovelRTSceneNode) = NULL;

int32_t Internal_BreadthFirstIteratorFunctionDelegate(const std::shared_ptr<SceneGraph::SceneNode>& node) {
  return _function(reinterpret_cast<NovelRTSceneNode>(node.get()));
}

#ifdef __cplusplus
extern "C" {
#endif

int32_t NovelRT_SceneNodeBreadthFirstIterator_create(NovelRTSceneNode node, int32_t(*func)(NovelRTSceneNode), NovelRTSceneNodeBreadthFirstIterator* outputIterator) {
  if(node == nullptr|| func == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }
    
  auto nodePointer = reinterpret_cast<SceneGraph::SceneNode*>(node)->shared_from_this();

  _function = func;
  SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t> iterator = SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>(nodePointer, Internal_BreadthFirstIteratorFunctionDelegate);
  *outputIterator = reinterpret_cast<NovelRTSceneNodeBreadthFirstIterator>(&iterator); 
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_increment(NovelRTSceneNodeBreadthFirstIterator iterator) {
  if(iterator == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }

  auto cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
  cppIterator->operator++();
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_postFixIncrement(NovelRTSceneNodeBreadthFirstIterator iterator) {
  if(iterator == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }

  auto cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
  ++cppIterator; //TODO: is this correct lol?
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_isEnd(NovelRTSceneNodeBreadthFirstIterator iterator, int32_t* outputResult) {
  if(iterator == nullptr || outputResult == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }
      
  SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>* cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
      
  *outputResult = cppIterator->isEnd();
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_isEqual(NovelRTSceneNodeBreadthFirstIterator iterator, NovelRTSceneNodeBreadthFirstIterator other, int32_t* outputResult) {
  if(iterator == nullptr || outputResult == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }

  auto cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
  auto otherCppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(other);

  *outputResult = (*cppIterator == *otherCppIterator);
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_isNotEqual(NovelRTSceneNodeBreadthFirstIterator iterator, NovelRTSceneNodeBreadthFirstIterator other, int32_t* outputResult) {
  if(iterator == nullptr || outputResult == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }

  auto cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
  auto otherCppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(other);

  *outputResult = (*cppIterator != *otherCppIterator);
  return NOVELRT_SUCCESS;
}

int32_t NovelRT_SceneNodeBreadthFirstIterator_runFunction(NovelRTSceneNodeBreadthFirstIterator iterator, int32_t* outputResult) {
  if(iterator == nullptr || outputResult == nullptr) {
    NovelRT_setErrMsgIsNullptrInternal();
    return NOVELRT_FAILURE;
  }

  auto cppIterator = reinterpret_cast<SceneGraph::SceneNode::breadth_first_traversal_result_iterator<int32_t>*>(iterator);
  *outputResult = cppIterator->operator*();
  return NOVELRT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
