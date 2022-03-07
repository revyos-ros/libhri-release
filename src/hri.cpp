// Copyright 2022 PAL Robotics S.L.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the PAL Robotics S.L. nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "hri/hri.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
#include <utility>
#include "hri/body.h"
#include "hri/face.h"
#include "hri_msgs/IdsList.h"

using namespace std;
using namespace hri;

HRIListener::HRIListener()
{
  init();
}

HRIListener::~HRIListener()
{
  ROS_DEBUG("Closing the HRI Listener");

  faces.clear();

  for (auto& sub : feature_subscribers_)
  {
    sub.second.shutdown();
  }
}


map<ID, FaceWeakConstPtr> HRIListener::getFaces() const
{
  map<ID, FaceWeakConstPtr> result;

  // creates a map of *weak* pointers from the internally managed list of
  // shared pointers
  for (auto const& f : faces)
  {
    result[f.first] = f.second;
  }

  return result;
}

map<ID, BodyWeakConstPtr> HRIListener::getBodies() const
{
  map<ID, BodyWeakConstPtr> result;

  // creates a map of *weak* pointers from the internally managed list of
  // shared pointers
  for (auto const& f : bodies)
  {
    result[f.first] = f.second;
  }

  return result;
}

map<ID, VoiceWeakConstPtr> HRIListener::getVoices() const
{
  map<ID, VoiceWeakConstPtr> result;

  // creates a map of *weak* pointers from the internally managed list of
  // shared pointers
  for (auto const& f : voices)
  {
    result[f.first] = f.second;
  }

  return result;
}

map<ID, PersonConstPtr> HRIListener::getPersons() const
{
  return persons;
}

void HRIListener::init()
{
  ROS_DEBUG("Initialising the HRI Listener");

  feature_subscribers_[FeatureType::face] = node_.subscribe<hri_msgs::IdsList>(
      "/humans/faces/tracked", 1,
      bind(&HRIListener::onTrackedFeature, this, FeatureType::face, _1));

  feature_subscribers_[FeatureType::body] = node_.subscribe<hri_msgs::IdsList>(
      "/humans/bodies/tracked", 1,
      bind(&HRIListener::onTrackedFeature, this, FeatureType::body, _1));

  feature_subscribers_[FeatureType::voice] = node_.subscribe<hri_msgs::IdsList>(
      "/humans/voices/tracked", 1,
      bind(&HRIListener::onTrackedFeature, this, FeatureType::voice, _1));

  feature_subscribers_[FeatureType::person] = node_.subscribe<hri_msgs::IdsList>(
      "/humans/persons/tracked", 1,
      bind(&HRIListener::onTrackedFeature, this, FeatureType::person, _1));
}

void HRIListener::onTrackedFeature(FeatureType feature, hri_msgs::IdsListConstPtr tracked)
{
  // update the current list of tracked feature (face, body...) with
  // what has just been received on the respective /tracked topic.
  
  set<ID> new_ids;
  for (auto const& id : tracked->ids)
  {
    new_ids.insert(ID(id));
  }

  set<ID> to_remove;
  set<ID> to_add;

  set<ID> current_ids;

  switch (feature)
  {
    case FeatureType::face:
      for (auto const& kv : faces)
      {
        current_ids.insert(kv.first);
      }
      break;
    case FeatureType::body:
      for (auto const& kv : bodies)
      {
        current_ids.insert(kv.first);
      }
      break;
    case FeatureType::voice:
      for (auto const& kv : voices)
      {
        current_ids.insert(kv.first);
      }
      break;
    case FeatureType::person:
      for (auto const& kv : persons)
      {
        current_ids.insert(kv.first);
      }
      break;
  }



  for (auto id : new_ids)
  {
    if (current_ids.find(id) == current_ids.end())
    {
      to_add.insert(id);
    }
  }

  for (auto id : current_ids)
  {
    if (new_ids.find(id) == new_ids.end())
    {
      to_remove.insert(id);
    }
  }

  switch (feature)
  {
    case FeatureType::face:
      for (auto id : to_remove)
      {
        faces.erase(id);
      }
      break;
    case FeatureType::body:
      for (auto id : to_remove)
      {
        bodies.erase(id);
      }
      break;
    case FeatureType::voice:
      for (auto id : to_remove)
      {
        voices.erase(id);
      }
      break;
    case FeatureType::person:
      for (auto id : to_remove)
      {
        persons.erase(id);
      }
      break;
  }

  switch (feature)
  {
    case FeatureType::face:
      for (auto id : to_add)
      {
        auto face = make_shared<Face>(id, node_);
        face->init();
        faces.insert({ id, face });
      }
      break;
    case FeatureType::body:
      for (auto id : to_add)
      {
        auto body = make_shared<Body>(id, node_);
        body->init();
        bodies.insert({ id, body });
      }
      break;
    case FeatureType::voice:
      for (auto id : to_add)
      {
        auto voice = make_shared<Voice>(id, node_);
        voice->init();
        voices.insert({ id, voice });
      }
      break;
    case FeatureType::person:
      for (auto id : to_add)
      {
        auto person = make_shared<Person>(id, this, node_);
        person->init();
        persons.insert({ id, person });
      }
      break;
  }
}

