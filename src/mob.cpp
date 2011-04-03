/*
  Copyright (c) 2011, The Mineserver Project
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "mob.h"
// #include "math.h"   // ??? do you mean <math.h> ? -- louisdx 
#include <algorithm>

Mob::Mob()
  :
  UID(Mineserver::generateEID()),
  x(0),
  y(0),
  z(0),
  map(0),
  yaw(0),
  pitch(0),
  meta(0)
  // louidx: We MUST initialize spawned
{
}

Mob::~Mob()
{
  // louisdx: Does this destructor at all do what you want?
  //   Right now you are making a COPY of a vector and erase
  //   the elements of that copy. You are NOT deleting the objects
  //   pointed to by those elements.

  /*
    std::vector<Mob*> mobs = Mineserver::get()->mobs()->getAll();
    for (std::vector<Mob*>::iterator i = mobs.begin() ; i != mobs.end(); i++)
    {
      if ((*i)->UID == UID)
      {
      mobs.erase(i);
      break;
      }
    }
  */

  // louisdx: Suggested fix follows. I will change Mobs::getAll() as well.
  std::vector<Mob*>::iterator it = std::find(Mineserver::get()->mobs()->getAll().begin(),
                                             Mineserver::get()->mobs()->getAll().end(),
                                             this);
  if (it != Mineserver::get()->mobs()->getAll().end())
  {
    Mineserver::get()->mobs()->getAll().erase(it);
  }

}

void Mob::sethealth(int health)
{
  if (health < 0)
  {
    health = 0;
  }
  if (health > 30)
  {
    health = 30;
  }
  if (health < this->health)
  {
    for (size_t i = 0; i < Mineserver::get()->users().size(); i++)
    {
      User* user = Mineserver::get()->users()[i];
      user->buffer << (int8_t)PACKET_ARM_ANIMATION << (int32_t)UID <<
                   (int8_t)2 ;
      // Hurt animation
    }
  }
  this->health = health;
  if (this->health <= 0)
  {
    deSpawnToAll();
  }
}



void Mob::spawnToAll()
{
  if (type == MOB_PIG)
  {
    health = 10;
  }
  if (type == MOB_SHEEP)
  {
    health = 10;
  }
  if (type == MOB_COW)
  {
    health = 10;
  }
  if (type == MOB_CHICKEN)
  {
    health = 4;
  }
  if (type == MOB_SQUID)
  {
    health = 10;
  }
  for (size_t i = 0; i < Mineserver::get()->users().size(); i++)
  {
    User* user = Mineserver::get()->users()[i];
    if (user->logged)
    {
      user->buffer << (int8_t)PACKET_MOB_SPAWN << (int32_t) UID << (int8_t) type
                   << (int32_t)(x * 32.0) << (int32_t)(y * 32.0) << (int32_t)(z * 32.0) << (int8_t) yaw
                   << (int8_t) pitch;
      if (type == MOB_SHEEP)
      {
        user->buffer << (int8_t) 0 << (int8_t) meta << (int8_t) 127;
      }
      else
      {
        user->buffer << (int8_t) 127;
      }
    }
    spawned = true;
  }
}

void Mob::deSpawnToAll()
{
  for (size_t i = 0; i < Mineserver::get()->users().size(); i++)
  {
    User* user = Mineserver::get()->users()[i];
    if (user->logged)
    {
      user->buffer << PACKET_DESTROY_ENTITY << (int32_t) UID;
    }
  }
  spawned = false;
}

void Mob::relativeMoveToAll()
{

}

void Mob::teleportToAll()
{
  if (!spawned)
  {
    return;
  }
  for (size_t i = 0; i < Mineserver::get()->users().size(); i++)
  {
    User* user = Mineserver::get()->users()[i];
    if (user->logged)
    {
      user->buffer << PACKET_ENTITY_TELEPORT << (int32_t) UID
                   << (int32_t)(x * 32.0) << (int32_t)(y * 32.0) << (int32_t)(z * 32.0)
                   << (int8_t) yaw << (int8_t) pitch;
    }
  }
}

void Mob::moveTo(double to_x, double to_y, double to_z, int to_map)
{
  //  int distx = abs(x-to_x);
  //  int disty = abs(y-to_y);
  //  int distz = abs(z-to_z);
  x = to_x;
  y = to_y;
  z = to_z;
  if (to_map != -1)
  {
    map = to_map;
  }
  //  if(distx < 4 && disty < 4 && distz < 4)
  //  {
  //    // Work out how to use the relative move?
  //    teleportToAll();
  //  }
  //  else
  //  {
  teleportToAll();
  //  }
}

void Mob::look(int16_t yaw, int16_t pitch)
{
  // Yaw and Pitch need to be between 0 and 360
  while (yaw < 0)
  {
    yaw += 360;
  }
  while (pitch < 0)
  {
    pitch += 360;
  }
  yaw = yaw % 360;
  pitch = pitch % 360;
  int8_t y_byte = (int8_t)((yaw * 1.0) / 360.0 * 256.0);
  int8_t p_byte = (int8_t)((pitch * 1.0) / 360.0 * 256.0);
  this->pitch = p_byte;
  this->yaw = y_byte;
  Packet pkt;
  pkt << PACKET_ENTITY_LOOK << (int32_t) UID << (int8_t) y_byte << (int8_t) p_byte;
  if (User::all().size() > 0)
  {
    User::all()[0]->sendAll((uint8_t*)pkt.getWrite(), pkt.getWriteLen());
  }
}


Mob* Mobs::getMobByID(int id)
{
  if (int(m_moblist.size()) <= id || id < 0)
  {
    return NULL;
  }
  return m_moblist[id];
}

// louisdx: may be easier to use an std::tr1::unordered_map<std::string, int>
int Mobs::mobNametoType(std::string name)
{
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  if (name == "CREEPER")
  {
    return MOB_CREEPER;
  }
  if (name == "SKELETON")
  {
    return MOB_SKELETON;
  }
  if (name == "SPIDER")
  {
    return MOB_SPIDER;
  }
  if (name == "GIANTZOMBIE")
  {
    return MOB_GIANT_ZOMBIE;
  }
  if (name == "GIANT")
  {
    return MOB_GIANT_ZOMBIE;
  }
  if (name == "ZOMBIE")
  {
    return MOB_ZOMBIE;
  }
  if (name == "SLIME")
  {
    return MOB_SLIME;
  }
  if (name == "GHAST")
  {
    return MOB_GHAST;
  }
  if (name == "ZOMBIEPIGMAN")
  {
    return MOB_ZOMBIE_PIGMAN;
  }
  if (name == "PIGMAN")
  {
    return MOB_ZOMBIE_PIGMAN;
  }
  if (name == "PIG")
  {
    return MOB_PIG;
  }
  if (name == "SHEEP")
  {
    return MOB_SHEEP;
  }
  if (name == "COW")
  {
    return MOB_COW;
  }
  if (name == "CHICKEN")
  {
    return MOB_CHICKEN;
  }
  return 0;
}
