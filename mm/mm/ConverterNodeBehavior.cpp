//
//  ConverterNodeBehavior.cpp
//  mm
//
//  Created by Riemer van Rozen on 11/21/13.
//  Copyright (c) 2013 Riemer van Rozen. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "YYLTYPE.h"
#include "Types.h"
#include "Recyclable.h"
#include "Vector.h"
#include "Map.h"
#include "Recycler.h"
#include "Observer.h"
#include "Observable.h"
#include "Location.h"
#include "String.h"
#include "Name.h"
#include "Element.h"
#include "Operator.h"
#include "Exp.h"
#include "Assertion.h"
#include "Deletion.h"
#include "Activation.h"
#include "Signal.h"
#include "Edge.h"
#include "StateEdge.h"
#include "FlowEdge.h"
#include "NodeWorkItem.h"
#include "NodeBehavior.h"
#include "Node.h"
#include "Transformation.h"
#include "Modification.h"
#include "Transition.h"
#include "FlowEvent.h"
#include "Program.h"
#include "PoolNodeBehavior.h"
#include "SourceNodeBehavior.h"
#include "DrainNodeBehavior.h"
#include "RefNodeBehavior.h"
#include "GateNodeBehavior.h"
#include "ConverterNodeBehavior.h"
#include "Declaration.h"
#include "InterfaceNode.h"
#include "Definition.h"
#include "Instance.h"
#include "Operator.h"
#include "ValExp.h"
#include "UnExp.h"
#include "BinExp.h"
#include "RangeValExp.h"
#include "BooleanValExp.h"
#include "NumberValExp.h"
#include "OverrideExp.h"
#include "ActiveExp.h"
#include "AllExp.h"
#include "DieExp.h"
#include "AliasExp.h"
#include "OneExp.h"
#include "VarExp.h"
#include "Reflector.h"
#include "Evaluator.h"
#include "Machine.h"

const MM::CHAR * MM::ConverterNodeBehavior::CONVERTER_STR = "converter";
const MM::CHAR * MM::ConverterNodeBehavior::FROM_STR = "from";
const MM::CHAR * MM::ConverterNodeBehavior::TO_STR  = "to";
const MM::UINT32 MM::ConverterNodeBehavior::CONVERTER_LEN =
  strlen(MM::ConverterNodeBehavior::CONVERTER_STR);
const MM::UINT32 MM::ConverterNodeBehavior::FROM_LEN =
  strlen(MM::ConverterNodeBehavior::FROM_STR);
const MM::UINT32 MM::ConverterNodeBehavior::TO_LEN  =
  strlen(MM::ConverterNodeBehavior::TO_STR);


/*
 Nodes behaving like converters share their edges
 with the implementing source and drain nodes.
 This makes updating them sort of tricky: see Reflector.
 Also, converter nodes are not scheduled,
 instead the definition contains the source and the drain as anonymous nodes,
 such that the semantics of those node behaviors is reused.
 */
MM::ConverterNodeBehavior::ConverterNodeBehavior(MM::NodeBehavior::IO   io,
                                                 MM::NodeBehavior::When when,
                                                 MM::Name * from,
                                                 MM::Name * to) :
  MM::NodeBehavior(io,
                   when,
                   MM::NodeBehavior::ACT_PULL,
                   MM::NodeBehavior::HOW_ALL)
{
  this->from = from;
  this->to = to;
  this->sourceNode = MM_NULL;
  this->drainNode = MM_NULL;
  this->triggerEdge = MM_NULL;
}

MM::ConverterNodeBehavior::~ConverterNodeBehavior()
{
  this->from = MM_NULL;
  this->to = MM_NULL;
  this->sourceNode = MM_NULL;
  this->drainNode = MM_NULL;
  this->triggerEdge = MM_NULL;
}

MM::VOID MM::ConverterNodeBehavior::recycle(MM::Recycler *r)
{
  if(from != MM_NULL)
  {
    from->recycle(r);
  }
  if(to != MM_NULL)
  {
    to->recycle(r);
  }
  MM::NodeBehavior::recycle(r);
}

MM::TID MM::ConverterNodeBehavior::getTypeId()
{
  return MM::T_ConverterNodeBehavior;
}

MM::BOOLEAN MM::ConverterNodeBehavior::instanceof(MM::TID tid)
{
  if(tid == MM::T_ConverterNodeBehavior)
  {
    return MM_TRUE;
  }
  else
  {
    return MM::NodeBehavior::instanceof(tid);
  }
}

MM::Name * MM::ConverterNodeBehavior::getFrom()
{
  return from;
}

MM::Name * MM::ConverterNodeBehavior::getTo()
{
  return to;
}

MM::VOID MM::ConverterNodeBehavior::setTriggerEdge(MM::Edge * triggerEdge)
{
  this->triggerEdge = triggerEdge;
}

MM::VOID MM::ConverterNodeBehavior::setSourceNode(MM::Node * sourceNode)
{
  this->sourceNode = sourceNode;
}

MM::VOID MM::ConverterNodeBehavior::setDrainNode(MM::Node * drainNode)
{
  this->drainNode = drainNode;
}

MM::Edge * MM::ConverterNodeBehavior::getTriggerEdge()
{
  return triggerEdge;
}

MM::Node * MM::ConverterNodeBehavior::getSourceNode()
{
  return sourceNode;
}

MM::Node * MM::ConverterNodeBehavior::getDrainNode()
{
  return drainNode;
}

MM::VOID MM::ConverterNodeBehavior::setFrom(MM::Name * from)
{
  this->from = from;
}

MM::VOID MM::ConverterNodeBehavior::setTo(MM::Name * to)
{
  this->to = to;
}

MM::UINT32 MM::ConverterNodeBehavior::getCreateMessage()
{
  return MM::MSG_NEW_CONVERTER;
}

MM::UINT32 MM::ConverterNodeBehavior::getUpdateMessage()
{
  return MM::MSG_UPD_CONVERTER;
}

MM::UINT32 MM::ConverterNodeBehavior::getDeleteMessage()
{
  return MM::MSG_DEL_CONVERTER;
}

MM::VOID MM::ConverterNodeBehavior::step(MM::Node * n,
              MM::Instance * i,
              MM::Machine * m,
              MM::Transition * t)
{
  //don't step --> drain / source
}

MM::VOID MM::ConverterNodeBehavior::stepPullAll(MM::Node * node,
                                                MM::Instance * i,
                                                MM::Vector<MM::NodeWorkItem *> * work,
                                                MM::Machine * m,
                                                MM::Transition * tr)
{
  //don't pull all --> drain / source
}

MM::VOID MM::ConverterNodeBehavior::stepPushAny(MM::Node * node,
                                                MM::Instance * i,
                                                MM::Vector<MM::NodeWorkItem *> * work,
                                                MM::Machine * m,
                                                MM::Transition * tr)
{
  //don't push all --> drain / source
}


MM::VOID MM::ConverterNodeBehavior::stepPullAny(MM::Node * node,
                     MM::Instance * i,
                     MM::Vector<MM::NodeWorkItem *> * work,
                     MM::Machine * m,
                     MM::Transition * tr)
{
  //don't pull all --> drain / source
}

MM::VOID MM::ConverterNodeBehavior::stepPushAll(MM::Node * node,
                     MM::Instance * i,
                     MM::Vector<MM::NodeWorkItem *> * work,
                     MM::Machine * m,
                     MM::Transition * tr)
{
  //don't push all --> drain / source
}


MM::VOID MM::ConverterNodeBehavior::begin(MM::Instance * i,
                                          MM::Machine * m,
                                          MM::Node * n)
{
  //do nothing
}

MM::VOID MM::ConverterNodeBehavior::end(MM::Instance * i,
                                        MM::Machine * m,
                                        MM::Node * n)
{
  //do nothing
}

MM::VOID MM::ConverterNodeBehavior::change(MM::Instance * i,
                                           MM::Machine * m,
                                           MM::Node * n)
{
  //TODO
}

MM::VOID MM::ConverterNodeBehavior::add(MM::Instance * i,
                                        MM::Machine * m,
                                        MM::Node * n,
                                        MM::UINT32 amount)
{
  return drainNode->add(i, m, amount);
}

MM::VOID MM::ConverterNodeBehavior::sub(MM::Instance * i,
                                        MM::Machine * m,
                                        MM::Node * n,
                                        MM::UINT32 amount)
{
  return sourceNode->sub(i, m, amount);
}

MM::UINT32 MM::ConverterNodeBehavior::getCapacity(MM::Instance * i,
                                                  MM::Node * n)
{
  return drainNode->getCapacity(i);
}

MM::UINT32 MM::ConverterNodeBehavior::getResources(MM::Instance * i,
                                                   MM::Node * n)
{
  return sourceNode->getResources(i);
}

MM::BOOLEAN MM::ConverterNodeBehavior::hasCapacity(MM::Instance * i,
                                                   MM::Node * n,
                                                   MM::UINT32 amount)
{
  return drainNode->hasCapacity(i, amount);
}

MM::BOOLEAN MM::ConverterNodeBehavior::hasResources(MM::Instance * i,
                                                    MM::Node * n,
                                                    MM::UINT32 amount)
{
  return sourceNode->hasResources(i, amount);
}



MM::VOID MM::ConverterNodeBehavior::activateTriggerTargets(MM::Node * node,
                                                           MM::Instance * i,
                                                           MM::Machine * m)
{
  //trigger the source node
  //i->setActive(sourceNode);
  //MM::NodeBehavior::activateTriggerTargets(node, i, m);
}

MM::VOID MM::ConverterNodeBehavior::toString(MM::String * buf)
{
  buf->append((MM::CHAR*)MM::ConverterNodeBehavior::CONVERTER_STR,
              MM::ConverterNodeBehavior::CONVERTER_LEN);
}

MM::VOID MM::ConverterNodeBehavior::toString(MM::String * buf,
                                             MM::Name * name)
{
  MM::NodeBehavior::toString(buf, name);
  toString(buf);
  buf->space();
  name->toString(buf);
  buf->space();
  if(from != MM_NULL)
  {
    buf->append((MM::CHAR*)MM::ConverterNodeBehavior::FROM_STR,
                MM::ConverterNodeBehavior::FROM_LEN);
    buf->space();
    from->toString(buf);
    buf->space();
  }
  if(to != MM_NULL)
  {
    buf->append((MM::CHAR*)MM::ConverterNodeBehavior::TO_STR,
                MM::ConverterNodeBehavior::TO_LEN);
    buf->space();
    to->toString(buf);
    buf->space();
  }
}