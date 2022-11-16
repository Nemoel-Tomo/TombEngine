#include "framework.h"
#include "ObjectsHandler.h"

#include "ReservedScriptNames.h"
#include "Lara/lara.h"
#include "ObjectIDs.h"
#include "Camera/CameraObject.h"
#include "Sink/SinkObject.h"
#include "SoundSource/SoundSourceObject.h"
#include "collision/collide_item.h"
#include "collision/collide_room.h"
#include "ScriptInterfaceGame.h"
#include "Lara/LaraObject.h"

/***
Moveables, statics, cameras, and so on.
@tentable Objects 
@pragma nostrip
*/

ObjectsHandler::ObjectsHandler(sol::state* lua, sol::table & parent) :
	m_handler{ lua },
	m_table_objects(sol::table{m_handler.GetState()->lua_state(), sol::create})
{
	parent.set(ScriptReserved_Objects, m_table_objects);

	/***
	Get a moveable by its name.
	@function GetMoveableByName
	@tparam string name the unique name of the Moveable as set in, or generated by, Tomb Editor
	@treturn Moveable a non-owning Moveable referencing the item.
	*/
	m_table_objects.set_function(ScriptReserved_GetMoveableByName, &ObjectsHandler::GetByName<Moveable, ScriptReserved_Moveable>, this);

	/***
	Get a Static by its name.
	@function GetStaticByName
	@tparam string name the unique name of the mesh as set in, or generated by, Tomb Editor
	@treturn Static a non-owning Static referencing the mesh.
	*/
	m_table_objects.set_function(ScriptReserved_GetStaticByName, &ObjectsHandler::GetByName<Static, ScriptReserved_Static>, this);

	/***
	Get moveables by its slot.
	@function GetMoveablesBySlot
	@tparam string slot the unique slot of the Moveable like ANIMATING1
	@treturn table of Moveables referencing the given slot.
	*/
	m_table_objects.set_function(ScriptReserved_GetMoveablesBySlot, &ObjectsHandler::GetMoveablesBySlot<Moveable>, this);

	/***
	Get statics by its slot.
	@function GetStaticsById
	@tparam int slot the unique slot of the mesh like 10
	@treturn table of Statics referencing the given slot id.
	*/
	m_table_objects.set_function(ScriptReserved_GetStaticsBySlot, &ObjectsHandler::GetStaticsBySlot<Static>, this);

	/***
	Get a Camera by its name.
	@function GetCameraByName
	@tparam string name the unique name of the camera as set in, or generated by, Tomb Editor
	@treturn Camera a non-owning Camera referencing the camera.
	*/
	m_table_objects.set_function(ScriptReserved_GetCameraByName, &ObjectsHandler::GetByName<Camera, ScriptReserved_Camera>, this);

	/***
	Get a Sink by its name.
	@function GetSinkByName
	@tparam string name the unique name of the sink as set in, or generated by, Tomb Editor
	@treturn Sink a non-owning Sink referencing the sink.
	*/
	m_table_objects.set_function(ScriptReserved_GetSinkByName, &ObjectsHandler::GetByName<Sink, ScriptReserved_Sink>, this);

	/***
	Get a SoundSource by its name.
	@function GetSoundSourceByName
	@tparam string name the unique name of the sound source as set in, or generated by, Tomb Editor
	@treturn SoundSource a non-owning SoundSource referencing the sound source.
	*/
	m_table_objects.set_function(ScriptReserved_GetSoundSourceByName, &ObjectsHandler::GetByName<SoundSource, ScriptReserved_SoundSource>, this);

	/***
	Get an AIObject by its name.
	@function GetAIObjectByName
	@tparam string name the unique name of the AIObject as set in, or generated by, Tomb Editor
	@treturn AIObject a non-owning SoundSource referencing the AI moveable.
	*/
	m_table_objects.set_function(ScriptReserved_GetAIObjectByName, &ObjectsHandler::GetByName<AIObject, ScriptReserved_AIObject>, this);

	LaraObject::Register(m_table_objects);

	Moveable::Register(m_table_objects);
	Moveable::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	Static::Register(m_table_objects);
	Static::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	Camera::Register(m_table_objects);
	Camera::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	Sink::Register(m_table_objects);
	Sink::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	AIObject::Register(m_table_objects);
	AIObject::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	SoundSource::Register(m_table_objects);
	SoundSource::SetNameCallbacks(
		[this](auto && ... param) { return AddName(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return RemoveName(std::forward<decltype(param)>(param)...); }
	);

	m_handler.MakeReadOnlyTable(m_table_objects, ScriptReserved_ObjID, kObjIDs);
}

void ObjectsHandler::TestCollidingObjects()
{
	// remove any items which can't collide
	for (const auto id : m_collidingItemsToRemove)
	{
		m_collidingItems.erase(id);
	}
	m_collidingItemsToRemove.clear();

	for (const auto idOne : m_collidingItems)
	{
		auto item = &g_Level.Items[idOne];
		if (!item->Callbacks.OnObjectCollided.empty())
		{
			//test against other moveables
			GetCollidedObjects(item, 0, true, CollidedItems, nullptr, 0);
			size_t i = 0;
			while (CollidedItems[i])
			{
				short idTwo = CollidedItems[i] - &g_Level.Items[0];
				g_GameScript->ExecuteFunction(item->Callbacks.OnObjectCollided, idOne, idTwo);
				++i;
			}
		}

		if (!item->Callbacks.OnRoomCollided.empty())
		{
			//test against room geometry
			if (TestItemRoomCollisionAABB(item))
			{
				g_GameScript->ExecuteFunction(item->Callbacks.OnRoomCollided, idOne);
			}
		}
	}
}

void ObjectsHandler::AssignLara()
{
	m_table_objects.set(ScriptReserved_Lara, LaraObject(Lara.ItemNumber, false));
}


bool ObjectsHandler::NotifyKilled(ItemInfo* key)
{
	auto it = m_moveables.find(key);
	if (std::end(m_moveables) != it)
	{
		for (auto& m : m_moveables[key])
		{
			m->Invalidate();
		}
		return true;
	}
	return false;
}

bool ObjectsHandler::AddMoveableToMap(ItemInfo* key, Moveable* mov)
{
	std::unordered_set<Moveable*> movVec;
	movVec.insert(mov);
	auto it = m_moveables.find(key);
	if (std::end(m_moveables) == it)
	{
		return m_moveables.insert(std::pair{ key, movVec }).second;
	}
	else
	{
		m_moveables[key].insert(mov);
		return true;
	}
}

bool ObjectsHandler::RemoveMoveableFromMap(ItemInfo* key, Moveable* mov)
{
	auto it = m_moveables.find(key);
	if (std::end(m_moveables) != it)
	{
		auto& set = m_moveables[key];
		bool erased = static_cast<bool>(set.erase(mov));
		if (erased && set.empty())
		{
			erased = erased && static_cast<bool>(m_moveables.erase(key));
		}
		return erased;
	}
	return false;
}


