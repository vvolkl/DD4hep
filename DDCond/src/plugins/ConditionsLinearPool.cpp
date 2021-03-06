// $Id$
//==========================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================
#ifndef DDCOND_CONDITIONSLINEARPOOL_H
#define DDCOND_CONDITIONSLINEARPOOL_H

// Framework include files
#include "DD4hep/objects/ConditionsInterna.h"
#include "DDCond/ConditionsPool.h"
#include "DDCond/ConditionsManager.h"
#include "DDCond/ConditionsSelectors.h"

// C/C++ include files
#include <list>

/// Namespace for the AIDA detector description toolkit
namespace DD4hep {

  /// Namespace for the geometry part of the AIDA detector description toolkit
  namespace Conditions {

    /// Class implementing the conditions collection for a given IOV type
    /**
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_CONDITIONS
     */
    template<typename MAPPING, typename BASE> 
    class ConditionsLinearPool : public BASE   {
    protected:
      typedef BASE      Base;
      typedef MAPPING   Mapping;
      typedef typename  BASE::key_type key_type;
      Mapping           m_entries;

    public:
      /// Default constructor
      ConditionsLinearPool(ConditionsManager mgr);

      /// Default destructor
      virtual ~ConditionsLinearPool();

      /// Total entry count
      virtual size_t count()  const   {
        return m_entries.size();
      }

      /// Full cleanup of all managed conditions.
      virtual void clear()   {
        for_each(m_entries.begin(), m_entries.end(), ConditionsPoolRemove(*this));
        m_entries.clear();
      }

      /// Check if a condition exists in the pool
      virtual Condition exists(Condition::key_type key)  const   {
        typename Mapping::const_iterator i=
          find_if(m_entries.begin(), m_entries.end(), HashConditionFind(key));
        return i==m_entries.end() ? Condition() : (*i);
      }

      /// Register a new condition to this pool
      virtual void insert(Condition condition)
      {  m_entries.insert(m_entries.end(),condition.access());      }

      /// Register a new condition to this pool. May overload for performance reasons.
      virtual void insert(RangeConditions& new_entries)
      {  for_each(new_entries.begin(), new_entries.end(), collectionSelect(m_entries));   }

      /// Select the conditions matching the DetElement and the conditions name
      virtual void select(Condition::key_type key, RangeConditions& result)
      {  for_each(m_entries.begin(), m_entries.end(), keyedSelect(key, result));  }

      /// Select the conditons, used also by the DetElement of the condition
      virtual void select_used(RangeConditions& result)
      {  for_each(m_entries.begin(), m_entries.end(), activeSelect(result));  }

      /// Select the conditons, used also by the DetElement of the condition
      virtual void select_all(RangeConditions& result)
      {  for_each(m_entries.begin(), m_entries.end(), collectionSelect(result));  }

      /// Select the conditons, used also by the DetElement of the condition
      virtual void select_all(ConditionsPool& selection_pool) 
      {  for_each(m_entries.begin(), m_entries.end(), ConditionsPoolInsert(selection_pool));  }
    };

    /// Class implementing the conditions update pool for a given IOV type
    /**
     *
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_CONDITIONS
     */
    template<typename MAPPING, typename BASE> class ConditionsLinearUpdatePool 
      : public ConditionsLinearPool<MAPPING,BASE>
    {
      typedef typename ConditionsLinearPool<MAPPING,BASE>::key_type key_type;
    public:
      /// Default constructor
      ConditionsLinearUpdatePool(ConditionsManager mgr)
        : ConditionsLinearPool<MAPPING,BASE>(mgr) 
      {
      }

      /// Default destructor
      virtual ~ConditionsLinearUpdatePool()  {}

      /// Adopt all entries sorted by IOV. Entries will be removed from the pool
      virtual void popEntries(UpdatePool::UpdateEntries& entries)   {
        MAPPING& m = this->ConditionsLinearPool<MAPPING,BASE>::m_entries;
        if ( !m.empty() )  {
          for(typename MAPPING::iterator i=m.begin(); i!=m.end(); ++i)   {
            Condition::Object* o = *i;
            entries[o->iov].push_back(Condition(o));
          }
          m.clear();        
        }
      }

      /// Select the conditions matching the DetElement and the conditions name
      virtual void select_range(Condition::key_type key,
                                const Condition::iov_type& req, 
                                RangeConditions& result)
      {
        MAPPING& m = this->ConditionsLinearPool<MAPPING,BASE>::m_entries;
        if ( !m.empty() )   {
          unsigned int req_typ = req.iovType ? req.iovType->type : req.type;
          const IOV::Key& req_key = req.key();
          for(typename MAPPING::const_iterator i=m.begin(); i != m.end(); ++i)  {
            if ( key == (*i)->hash )  {
              const IOV* _iov = (*i)->iov;
              unsigned int typ = _iov->iovType ? _iov->iovType->type : _iov->type;
              if ( req_typ == typ )   {
                if ( IOV::key_is_contained(_iov->key(),req_key) )
                  // IOV test contained in key. Take it!
                  result.push_back(*i);
                else if ( IOV::key_overlaps_lower_end(_iov->key(),req_key) )
                  // IOV overlap on test on the lower end of key
                  result.push_back(*i);
                else if ( IOV::key_overlaps_higher_end(_iov->key(),req_key) )
                  // IOV overlap of test on the higher end of key
                  result.push_back(*i);
              }
            }
          }
        }
      }
 
    };


  } /* End namespace Conditions             */
} /* End namespace DD4hep                   */
#endif // DDCOND_CONDITIONSLINEARPOOL_H

// $Id$
//==========================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : M.Frank
//
//==========================================================================

// Framework include files
//#include "DDCond/ConditionsLinearPool.h"
#include "DD4hep/Printout.h"
#include "DD4hep/InstanceCount.h"

using namespace DD4hep::Conditions;

/// Default constructor
template<typename MAPPING, typename BASE>
ConditionsLinearPool<MAPPING,BASE>::ConditionsLinearPool(ConditionsManager mgr) : BASE(mgr)  {
  InstanceCount::increment(this);
}

/// Default destructor
template<typename MAPPING, typename BASE>
ConditionsLinearPool<MAPPING,BASE>::~ConditionsLinearPool()  {
  clear();
  InstanceCount::decrement(this);
}

/// Namespace for the AIDA detector description toolkit
namespace DD4hep {

  /// Namespace for the geometry part of the AIDA detector description toolkit
  namespace Conditions {

  } /* End namespace Conditions             */
} /* End namespace DD4hep                   */

#include "DD4hep/Factories.h"
namespace {
  ConditionsManager _mgr(int argc, char** argv)  {
    if ( argc > 0 )  {
      ConditionsManager::Object* m = (ConditionsManager::Object*)argv[0];
      return m;
    }
    DD4hep::except("ConditionsLinearPool","++ Insufficient arguments: arg[0] = ConditionManager!");
    return ConditionsManager(0);
  }

#define _CR(fun,x,b,y) void* fun(DD4hep::Geometry::LCDD&, int argc, char** argv) \
  {  return new b<x<Condition::Object*>,y>(_mgr(argc,argv));  }
  /// Create a conditions pool based on STL vectors
  _CR(create_vector_pool,std::vector,ConditionsLinearPool,ConditionsPool)
  /// Create a conditions pool based on STL lists
  _CR(create_list_pool,std::list,ConditionsLinearPool,ConditionsPool)
  /// Create a conditions update pool based on STL vectors
  _CR(create_vector_update_pool,std::vector,ConditionsLinearUpdatePool,UpdatePool)
  /// Create a conditions update pool based on STL list
  _CR(create_list_update_pool,std::list,ConditionsLinearUpdatePool,UpdatePool)
}

DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearPool,            create_vector_pool)
DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearVectorPool,      create_vector_pool)
DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearUpdatePool,      create_vector_update_pool)
DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearVectorUpdatePool,create_vector_update_pool)

DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearListPool,        create_list_pool)
DECLARE_LCDD_CONSTRUCTOR(DD4hep_ConditionsLinearListUpdatePool,  create_list_update_pool)
