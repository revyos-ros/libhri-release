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


#ifndef HRI_HRI_H
#define HRI_HRI_H

#include <ros/ros.h>
#include <hri_msgs/IdsList.h>

#include <functional>
#include <map>
#include <memory>

#include "base.h"
#include "face.h"
#include "body.h"
#include "voice.h"
#include "person.h"
#include "ros/subscriber.h"


namespace hri
{
/** \brief Main entry point to libhri. This is most likely what you want to use.
 * Use example:
 *
 * ```cpp
 * ros::init(argc, argv, "test_libhri");
 * ros::NodeHandle nh;
 *
 * HRIListener hri_listener;
 *
 * while (ros::ok()) {
 *
 *   auto faces = hri_listener.getFaces();
 *
 *   for (auto const& face : faces)
 *   {
 *     cout << "Face " << face.first << " seen!";
 *   }
 * }
 * ```
 */
class HRIListener
{
public:
  HRIListener();

  ~HRIListener();

  /** \brief Returns the list of currently detected faces, mapped to their IDs
   *
   * Faces are returned as constant std::weak_ptr as they may disappear at any point.
   */
  std::map<ID, FaceWeakConstPtr> getFaces() const;

  /** \brief Registers a callback function, to be invoked everytime a new face
   * is detected.
   */
  void onFace(std::function<void(FaceWeakConstPtr)> callback)
  {
    face_callbacks.push_back(callback);
  }

  /** \brief Returns the list of currently detected bodies, mapped to their IDs
   *
   * Bodies are returned as constant std::weak_ptr as they may disappear at any point.
   */
  std::map<ID, BodyWeakConstPtr> getBodies() const;

  /** \brief Registers a callback function, to be invoked everytime a new body
   * is detected.
   */
  void onBody(std::function<void(BodyWeakConstPtr)> callback)
  {
    body_callbacks.push_back(callback);
  }

  /** \brief Returns the list of currently detected voices, mapped to their IDs
   *
   * Voices are returned as constant std::weak_ptr as they may disappear at any point.
   */
  std::map<ID, VoiceWeakConstPtr> getVoices() const;

  /** \brief Registers a callback function, to be invoked everytime a new voice
   * is detected.
   */
  void onVoice(std::function<void(VoiceWeakConstPtr)> callback)
  {
    voice_callbacks.push_back(callback);
  }


  /** \brief Returns the list of currently detected persons, mapped to their IDs
   */
  std::map<ID, PersonConstPtr> getPersons() const;

  /** \brief Registers a callback function, to be invoked everytime a new person
   * is detected.
   */
  void onPerson(std::function<void(PersonConstPtr)> callback)
  {
    person_callbacks.push_back(callback);
  }



private:
  ros::NodeHandle node_;

  void init();

  void onTrackedFeature(FeatureType feature, hri_msgs::IdsListConstPtr tracked);

  std::map<FeatureType, ros::Subscriber> feature_subscribers_;

  std::map<ID, FaceConstPtr> faces;
  std::vector<std::function<void(FaceWeakConstPtr)>> face_callbacks;

  std::map<ID, BodyConstPtr> bodies;
  std::vector<std::function<void(BodyWeakConstPtr)>> body_callbacks;

  std::map<ID, VoiceConstPtr> voices;
  std::vector<std::function<void(VoiceWeakConstPtr)>> voice_callbacks;

  std::map<ID, PersonConstPtr> persons;
  std::vector<std::function<void(PersonConstPtr)>> person_callbacks;
};

}  // namespace hri



#endif  // HRI_HRI_H
