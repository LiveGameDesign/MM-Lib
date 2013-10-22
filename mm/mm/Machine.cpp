//
//  Machine.cpp
//  mm
//
//  Created by Riemer van Rozen on 7/16/13.
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
#include "Location.h"
#include "String.h"
#include "Name.h"
#include "Element.h"
#include "Transformation.h"
#include "Program.h"
#include "Modification.h"
#include "Transition.h"
#include "Operator.h"
#include "Exp.h"
#include "Assertion.h"
#include "Deletion.h"
#include "Signal.h"
#include "Edge.h"
#include "StateEdge.h"
#include "FlowEdge.h"
#include "NodeBehavior.h"
#include "Node.h"
#include "PoolNodeBehavior.h"
#include "SourceNodeBehavior.h"
#include "DrainNodeBehavior.h"
#include "GateNodeBehavior.h"
#include "RefNodeBehavior.h"
#include "Observer.h"
#include "Observable.h"
#include "Declaration.h"
#include "Definition.h"
#include "Instance.h"
#include "Operator.h"
#include "ValExp.h"
#include "UnExp.h"
#include "BinExp.h"
#include "DieExp.h"
#include "RangeValExp.h"
#include "BooleanValExp.h"
#include "NumberValExp.h"
#include "OverrideExp.h"
#include "ActiveExp.h"
#include "AllExp.h"
#include "AliasExp.h"
#include "OneExp.h"
#include "VarExp.h"
#include "Reflector.h"
#include "Evaluator.h"
#include "Machine.h"

MM::Machine::Machine() : MM::Recycler()
{
  reflector = new MM::Reflector(this);
  evaluator = new MM::Evaluator(this);
  log = createProgram();
  MM::Vector<MM::Element*> * elements = createElementVector();
  type = createDefinition(MM_NULL, elements);
  
  
  inst = createInstance(MM_NULL, type, MM_NULL);
  type->addObserver(inst);
  reflector->init(type);
  
  //trick to get the model out
  MM::Modification * mod = createModification();
  mod->addElement(type);
  log->addTransformation(mod);
  
  
}

MM::Machine::~Machine()
{
  delete reflector;
  delete evaluator;
  
  //type->recycle(this); //is in the program!
  inst->recycle(this);
  log->recycle(this);
}

MM::TID MM::Machine::getTypeId()
{
  return MM::T_Machine;
}

MM::BOOLEAN MM::Machine::instanceof(MM::TID tid)
{
  if(tid == MM::T_Machine)
  {
    return MM_TRUE;
  }
  else
  {
    return MM::Recycler::instanceof(tid);
  }
}

/*
MM::UINT32 MM::Machine::addInstance (MM::UINT32   inst, //0 -> global scope
                                     MM::UINT32   type,
                                     MM::CHAR   * name)
{
  MM::UINT32 pools = 0; //defs[def]->getPoolCount();
  MM::UINT32 gates = 0; //defs[def]->getGateCount();
  MM::UINT32 insts = 0; //defs[def]->getInstanceCount();

  MM::UINT32 size
    = sizeof(Instance)
    + pools * sizeof(MM::INT32)
    + gates * 2 * sizeof(MM::INT32)
    + insts * sizeof(MM::UINT32);
  
  MM::Instance * i = (MM::Instance*) malloc(size);
  MM::UINT32 pos = curInst;
  i->pools = pools;
  i->gates = gates;
  i->insts = insts;
  
  curInst++;
  
  this->insts[pos] = i;
  return pos;
}
*/

/*
MM::Transition * MM::Machine::step()
{  
  //A. prepare
  //1. set temporary values (new, old) to val
  //   for each node -> reset()
  
  
  //B. activity
  //1. pull all nodes
  //2. pull any nodes
  //3. push all nodes
  //4. push any nodes
  
  //C. finalize
  //1. set temporary values (new, old) to zero
  //2. redistribute gates
  //3. set active nodes in next state
  //4. process trigger to activate nodes in next state
  
  return NULL;
}
*/

extern MM::Program * MM_parse(const MM::CHAR * input);
extern MM::Program * MM_parseFile(const MM::CHAR * input);

MM::Reflector * MM::Machine::getReflector()
{
  return reflector;
}

MM::Evaluator * MM::Machine::getEvaluator()
{
  return evaluator;
}

MM::Definition * MM::Machine::getDefinition()
{
  return type;
}

MM::Instance * MM::Machine::getInstance()
{
  return inst;
}

MM::VOID MM::Machine::eval (const MM::CHAR * input)
{
  MM::Program * program = MM_parseFile(input);
  MM::String * buf = createString(1024 * 100 * 32);
  program->toString(buf);
  buf->print();
  
  MM::Vector<Transformation *> * ts = program->getTransformations();
  MM::Vector<Transformation *>::Iterator i = ts->getIterator();
  while(i.hasNext() == MM_TRUE)
  {
    Transformation * t = i.getNext();
    if(t->instanceof(MM::T_Modification) == MM_TRUE)
    {
      reflector->merge((MM::Modification *) t);
      MM::Definition * def = reflector->getDefinition();
      buf->clear();
      def->toString(buf);
      buf->print();
      
    }
  }
  
  MM::Instance * inst = reflector->getInstance();

  buf->clear();
  inst->toString(buf);
  buf->print();
  
  
  for(int i = 0; i < 10; i ++)
  {
    MM::Transition * tr = evaluator->step();
    program->addTransformation(tr);
    buf->clear();
    inst->toString(buf);
    buf->print();
  }
  
  
  printf("\n\nRESULTS\n\n");
  buf->clear();
  program->toString(buf);
  buf->print();
  
  buf->recycle(this);
  program->recycle(this);
}

//MM::VOID MM::Machine::setTree (MM::Definition * def)
//{
  //this->def = def;
  //}

//MM::Element * MM::Machine::getTree()
//{
  //return def;
  //}

//MM::Definition * MM::Machine::getDefinition(MM::Name * name)
//{
//  return types->get(name);
//}

//MM::VOID MM::Machine::putDefinition(MM::Name * name, MM::Definition * def)
//{
  //types->put(name, def);
  //}

MM::Vector<MM::Transformation *> * MM::Machine::createTransformationVector()
{
  MM::Vector<MM::Transformation *> * v = new Vector<MM::Transformation *> ();
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Element *> * MM::Machine::createElementVector()
{
  MM::Vector<MM::Element *> * v = new MM::Vector<MM::Element*> ();
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Node *> * MM::Machine::createNodeVector()
{
  MM::Vector<MM::Node *> * v = new MM::Vector<MM::Node *> (); 
  //TODO: process vectors in recycler
  return v;
}

MM::Vector<MM::Edge *> * MM::Machine::createEdgeVector()
{
  MM::Vector<MM::Edge *> * v = new MM::Vector<MM::Edge *> ();
  //TODO: process vectors in recycler
  return v;  
}

MM::Map<MM::Name *, MM::Element *, MM::Name::Compare> * MM::Machine::createName2ElementMap()
{
  MM::Map<MM::Name *, MM::Element *, MM::Name::Compare> * n2e =
    new MM::Map<MM::Name *, MM::Element *, MM::Name::Compare>();
  //TODO: process maps in recylcer
  return n2e;
}

MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * MM::Machine::createName2NodeMap()
{
  MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * n2n =
  new MM::Map<MM::Name *, MM::Node *, MM::Name::Compare>();
  //TODO: process maps in recylcer
  return n2n;
}

MM::String * MM::Machine::createString(MM::UINT32 size)
{
  MM::CHAR * buffer = createBuffer(size);
  MM::String * str = new MM::String(buffer,size);
  MM::Recycler::create(str);
  return str;
}

MM::Location * MM::Machine::createLocation(YYLTYPE * loc)
{
  MM::Location * r = new MM::Location(loc->first_line,
                                      loc->first_column,
                                      loc->last_line,
                                      loc->last_column);
  MM::Recycler::create(r);
  return r;
}

MM::Name * MM::Machine::createName(MM::CHAR   * str,
                                   MM::UINT32 * len,
                                   MM::UINT32 * start,
                                   MM::UINT32 * end)
{
  MM::CHAR * buf = createBuffer(*len+1);
  strncpy(buf, str + *start, *len);
  MM::Name * n = new MM::Name(buf, *len);
  MM::Recycler::create(n);  
   *start += *len + 1;
  return n;
}

MM::VOID MM::Machine::eatWhiteSpace(MM::CHAR   * str,
                                    MM::UINT32 * start,
                                    MM::UINT32 * end)
{
  MM::BOOLEAN whitespace = MM_TRUE;
  while(whitespace == MM_TRUE)
  {
    switch(str[*start])
    {
      case ' ':
      case '\t':
      case '\f':
      case '\r':
        (*start)++;
        break;
      default:
        whitespace = MM_FALSE;
        break;
    }
    if(*start >= *end)
    {
      break;
    }
  }
}

MM::Name * MM::Machine::createName(MM::CHAR * str, YYLTYPE  * strLoc)
{
  MM::Name * name = MM_NULL;
  MM::Name * dotRoot = MM_NULL;
  MM::Name * colonRoot = MM_NULL;
  MM::Name * curName = MM_NULL;
  
  MM::UINT32 start = 0;
  MM::UINT32 end = strlen(str);

  do
  {
    MM::UINT32 len1 = strcspn(str + start, ".");
    MM::UINT32 len2 = strcspn(str + start, ":");
    if(len1 <= len2)
    {
      name = createName(str, &len1, &start, &end);
      if(curName != MM_NULL)
      {
        curName->setName(name);
      }
      if(dotRoot == MM_NULL)
      {
        dotRoot = name;
      }
      
      curName = name;
    }
    else
    {
      name = createName(str, &len2, &start, &end);
      if(curName != MM_NULL)
      {
        curName->setName(name);
      }
      if(dotRoot != MM_NULL)
      {
        colonRoot = dotRoot; 
      }
      else
      {
        colonRoot = name;
      }
      dotRoot = MM_NULL;
      curName = MM_NULL;
      eatWhiteSpace(str, &start, &end);
    }
    
  } while(start < end);

  MM::Location * loc = MM::Machine::createLocation(strLoc);
  dotRoot->setLocation(loc);
  dotRoot->setPreName(colonRoot);
  
  return dotRoot;
}

MM::Name * MM::Machine::createName(MM::Name * name)
{
  MM::CHAR * buf = name->getBuffer();
  MM::UINT32 len = name->getLength();
  
  MM::CHAR * buf2 = createBuffer(len);
  strncpy(buf2, buf, len);
  
  MM::Name * r = new MM::Name(buf2, len);
  
  MM::Recycler::create(r);
  
  return r;
}

MM::Program * MM::Machine::createProgram()
{
  MM::Vector<MM::Transformation *> *
  transformations = createTransformationVector();
  MM::Program * r = new MM::Program(transformations);
  MM::Recycler::create(r);
  return r;  
}

MM::Program * MM::Machine::createProgram(MM::Vector<MM::Transformation *> *
                                         transformations)
{
  MM::Program * r = new MM::Program(transformations);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification()
{
  MM::Vector<MM::Element *> * elements = createElementVector();
  MM::Modification * r = new MM::Modification(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification(MM::Vector<MM::Element *> *
                                                   elements)
{
  MM::Modification * r = new MM::Modification(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Modification * MM::Machine::createModification(MM::Vector<MM::Element *> *
                                                   elements,
                                                   YYLTYPE * modifyLoc)
{
  MM::Location * loc = createLocation(modifyLoc);
  MM::Modification * r = new MM::Modification(elements, loc);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition()
{
  MM::Vector<MM::Element *> * elements = createElementVector();
  
  MM::Transition * r = new MM::Transition(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition(MM::Vector<MM::Element *> *
                                               elements)
{
  MM::Transition * r = new MM::Transition(elements);
  MM::Recycler::create(r);
  return r;
}

MM::Transition * MM::Machine::createTransition(MM::Vector<MM::Element *> *
                                               elements,
                                               YYLTYPE * stepLoc)
{
  MM::Location * loc = createLocation(stepLoc);
  MM::Transition * r = new MM::Transition(elements, loc);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createSourceNode(MM::NodeBehavior::IO   io,
                                         MM::NodeBehavior::When when,
                                         MM::NodeBehavior::Act  act,
                                         MM::NodeBehavior::How  how,
                                         MM::Name             * name)
{
  MM::SourceNodeBehavior * behavior =
    new MM::SourceNodeBehavior(io,when,act,how);
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createDrainNode(MM::NodeBehavior::IO    io,
                                             MM::NodeBehavior::When  when,
                                             MM::NodeBehavior::Act   act,
                                             MM::NodeBehavior::How   how,
                                             MM::Name      * name)
{
  MM::DrainNodeBehavior * behavior =
    new MM::DrainNodeBehavior(io,when,act,how);
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name,behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createGateNode(MM::NodeBehavior::IO    io,
                                       MM::NodeBehavior::When  when,
                                       MM::NodeBehavior::Act   act,
                                       MM::NodeBehavior::How   how,
                                       MM::Name      * name)
{
  MM::GateNodeBehavior * behavior =
    new MM::GateNodeBehavior(io,when,act,how);
  MM::Recycler::create(behavior);
  
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createPoolNode(MM::NodeBehavior::IO    io,
                                           MM::NodeBehavior::When  when,
                                           MM::NodeBehavior::Act   act,
                                           MM::NodeBehavior::How   how,
                                           MM::Name      * name,
                                           MM::UINT32      at,
                                           MM::UINT32      max,
                                           MM::Exp       * exp)
{
  MM::PoolNodeBehavior * behavior =
    new MM::PoolNodeBehavior(io,when,act,how,at,max,exp);
  MM::Recycler::create(behavior);
  
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::Node * MM::Machine::createRefNode(MM::Name * name)
{
  MM::RefNodeBehavior * behavior = new RefNodeBehavior();
  MM::Recycler::create(behavior);
  MM::Node * r = new MM::Node(name, behavior);
  MM::Recycler::create(r);
  return r;
}

MM::StateEdge * MM::Machine::createStateEdge(MM::Name * name,
                                             MM::Name * src,
                                             MM::Exp  * exp,
                                             MM::Name * tgt)
{
  MM::StateEdge * r = new MM::StateEdge(name,src,exp,tgt);
  MM::Recycler::create(r);
  return r;
}

MM::FlowEdge * MM::Machine::createFlowEdge(MM::Name * name,
                                           MM::Name * src,
                                           MM::Exp  * exp,
                                           MM::Name * tgt)
{
  MM::FlowEdge * r = new MM::FlowEdge(name,src,exp,tgt);
  MM::Recycler::create(r);
  return r;
}

MM::Definition * MM::Machine::createDefinition()
{
  MM::Vector<Element*> * elements = createElementVector();  
  MM::Definition * r = new MM::Definition(MM_NULL, elements);
  MM::Recycler::create(r);
  return r;
}

MM::Definition * MM::Machine::createDefinition(MM::Name * name,
                                               MM::Vector<Element*> * elements)
{
  MM::Definition * r = new MM::Definition(name, elements);
  MM::Recycler::create(r);
  return r;
}


MM::Declaration * MM::Machine::createDeclaration(MM::Name * type,
                                                 MM::Name * name)
{
  MM::Map<MM::Name *, MM::Node *, MM::Name::Compare> * interfaces =
    createName2NodeMap();
  MM::Declaration * r = new MM::Declaration(type, name, interfaces);
  MM::Recycler::create(r);
  return r;
}

MM::Assertion * MM::Machine::createAssertion(YYLTYPE  * assertLoc,
                                             MM::Name * name,
                                             MM::Exp  * exp,
                                             MM::CHAR * msg)
{
  MM::Location * loc = MM::Machine::createLocation(assertLoc);
  MM::UINT32 len = strlen(msg);
  MM::CHAR * buf = MM::Recycler::createBuffer(len);
  strncpy(buf, msg, len);
  MM::Assertion * r = new MM::Assertion(name,exp,buf,loc);
  
  MM::Recycler::create(r);
  return r;
}

MM::Assertion * MM::Machine::createAssertion(MM::Name * name,
                                             MM::Exp  * exp,
                                             MM::CHAR * msg)
{
  MM::UINT32 len = strlen(msg);
  MM::CHAR * buf = MM::Recycler::createBuffer(len);
  strncpy(buf, msg, len);
  MM::Assertion * r = new MM::Assertion(name,exp,buf);
  
  MM::Recycler::create(r);
  return r;
}

MM::Deletion * MM::Machine::createDeletion(YYLTYPE * deleteLoc,
                                           MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(deleteLoc);
  MM::Deletion * r = new MM::Deletion(loc, name);
  MM::Recycler::create(r);
  return r;
}

MM::Deletion * MM::Machine::createDeletion(MM::Name * name)
{
  MM::Deletion * r = new MM::Deletion(name);
  MM::Recycler::create(r);
  return r;
}


MM::Signal * MM::Machine::createSignal(MM::Name * name)
{
  MM::Signal * r = new MM::Signal(name);
  MM::Recycler::create(r);
  return r;
}

MM::Signal * MM::Machine::createSignal(YYLTYPE * signalLoc,
                                       MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(signalLoc);
  MM::Signal * r = new MM::Signal(loc, name);
  MM::Recycler::create(r);
  return r;  
}

MM::UnExp * MM::Machine::createUnExp(MM::Operator::OP  op,
                                     YYLTYPE         * opLoc,
                                     MM::Exp         * exp)
{
  MM::Location * loc = MM::Machine::createLocation(opLoc);
  MM::UnExp * r = new MM::UnExp(op, exp, loc);
  MM::Recycler::create(r);
  return r;
}

MM::UnExp * MM::Machine::createUnExp(MM::Operator::OP  op,
                                     MM::Exp         * exp)
{
  MM::UnExp * r = new MM::UnExp(op, exp);
  MM::Recycler::create(r);
  return r;
}

MM::BinExp * MM::Machine::createBinExp(MM::Exp          * e1,
                                       MM::Operator::OP   op,
                                       YYLTYPE          * opLoc,
                                       MM::Exp          * e2)
{
  MM::Location * loc = MM::Machine::createLocation(opLoc);
  MM::BinExp * r = new MM::BinExp(e1,op,e2,loc);
  MM::Recycler::create(r);
  return r;
}


MM::BinExp * MM::Machine::createBinExp(MM::Exp          * e1,
                                       MM::Operator::OP   op,
                                       MM::Exp          * e2)
{
  MM::BinExp * r = new MM::BinExp(e1,op,e2);
  MM::Recycler::create(r);
  return r;
}

MM::OverrideExp * MM::Machine::createOverrideExp(MM::Exp * e)
{
  MM::OverrideExp * r = new MM::OverrideExp(e);
  MM::Recycler::create(r);
  return r;
}

MM::OverrideExp * MM::Machine::createOverrideExp(YYLTYPE * lparenLoc,
                                                 MM::Exp * e,
                                                 YYLTYPE * rparenLoc)
{
  MM::Location * loc1 = createLocation(lparenLoc);
  MM::Location * loc2 = createLocation(rparenLoc);
  MM::OverrideExp * r = new MM::OverrideExp(loc1, e, loc2);
  MM::Recycler::create(r);
  return r;
}

MM::RangeValExp * MM::Machine::createRangeValExp(MM::INT32   v1,
                                                 YYLTYPE   * v1Loc,
                                                 YYLTYPE   * rangeLoc,
                                                 MM::INT32   v2,
                                                 YYLTYPE   * v2Loc)
{
  MM::Location * v1_loc = MM::Machine::createLocation(v1Loc);
  MM::Location * v2_loc = MM::Machine::createLocation(v2Loc);
  MM::Location * range_loc = MM::Machine::createLocation(rangeLoc);
  MM::RangeValExp * r = new MM::RangeValExp(v1, v2, v1_loc, range_loc, v2_loc);
  MM::Recycler::create(r);
  return r;
}

MM::RangeValExp * MM::Machine::createRangeValExp(MM::INT32   v1,
                                                 MM::INT32   v2)
{
  MM::RangeValExp * r = new MM::RangeValExp(v1, v2);
  MM::Recycler::create(r);
  return r;
}

MM::NumberValExp * MM::Machine::createNumberValExp(MM::INT32  val,
                                                   YYLTYPE  * valLoc)
{
  MM::Location * loc = MM::Machine::createLocation(valLoc);  
  MM::NumberValExp * r = new MM::NumberValExp(val/100, val%100, loc);
  MM::Recycler::create(r);
  return r;
}

MM::NumberValExp * MM::Machine::createNumberValExp(MM::INT32 val)
{
  MM::NumberValExp * r = new MM::NumberValExp(val/100, val%100, MM_NULL);
  MM::Recycler::create(r);
  return r;
}


MM::BooleanValExp * MM::Machine::createBooleanValExp(MM::BOOLEAN val,
                                                     YYLTYPE * valLoc)
{
  MM::Location * loc = MM::Machine::createLocation(valLoc);
  MM::BooleanValExp * r = new MM::BooleanValExp(val, loc);
  MM::Recycler::create(r);
  return r;
};

MM::BooleanValExp * MM::Machine::createBooleanValExp(MM::BOOLEAN val)
{
  MM::BooleanValExp * r = new MM::BooleanValExp(val, MM_NULL);
  MM::Recycler::create(r);
  return r;
};

MM::AllExp * MM::Machine::createAllExp(YYLTYPE * allLoc)
{
  MM::Location * loc = MM::Machine::createLocation(allLoc);
  MM::AllExp * r = new MM::AllExp(loc);
  MM::Recycler::create(r);
  return r;
};

MM::ActiveExp * MM::Machine::createActiveExp(YYLTYPE  * activeLoc,
                                             MM::Name * name)
{
  MM::Location * loc = MM::Machine::createLocation(activeLoc);
  MM::ActiveExp * r = new MM::ActiveExp(name,loc);
  MM::Recycler::create(r);
  return r;
}

MM::AliasExp * MM::Machine::createAliasExp(YYLTYPE * aliasLoc)
{
  MM::Location * loc = createLocation(aliasLoc);
  MM::AliasExp * r = new MM::AliasExp(loc);
  MM::Recycler::create(r);
  return r;
}

MM::OneExp * MM::Machine::createOneExp(YYLTYPE * epsilonLoc)
{
  MM::Location * loc = createLocation(epsilonLoc);
  MM::OneExp * r = new MM::OneExp(loc);
  MM::Recycler::create(r);
  return r;
}

MM::VarExp * MM::Machine::createVarExp(MM::Name * name)
{
  MM::VarExp * r = new MM::VarExp(name);
  MM::Recycler::create(r);
  return r;
}

MM::Instance * MM::Machine::createInstance(MM::Instance * parent,
                                           MM::Definition * def,
                                           MM::Name * name)
{
  MM::Instance * instance = new MM::Instance(parent, def, name);
  MM::Recycler::create(instance);
  return instance;
}