#include <algorithm>
#include <vector>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "defines.h"
#include "Source.h"
#include "Decorations.h"
#include "Mixer.h"
#include "Log.h"

#include "MixingGroup.h"

// utility to sort Sources in MixingView in a clockwise order
// in reference to a center point
struct clockwise_centered {
    clockwise_centered(glm::vec2 c) : center(c) { }
    bool operator() (Source * first, Source * second) {
        glm::vec2 pos_first = glm::vec2(first->group(View::MIXING)->translation_)-center;
        float angle_first = glm::orientedAngle( glm::normalize(pos_first), glm::vec2(1.f, 0.f) );
        glm::vec2 pos_second = glm::vec2(second->group(View::MIXING)->translation_)-center;
        float angle_second = glm::orientedAngle( glm::normalize(pos_second), glm::vec2(1.f, 0.f) );
        return (angle_first < angle_second);
    }
    glm::vec2 center;
};

MixingGroup::MixingGroup (SourceList sources) : root_(nullptr), lines_(nullptr), center_(nullptr),
    center_pos_(glm::vec2(0.f, 0.f)), active_(true)
{
    // fill the vector of sources with the given list
    for (auto it = sources.begin(); it != sources.end(); it++){
        (*it)->mixinggroup_ = this;
        sources_.push_back(*it);
        // compute barycenter (1)
        center_pos_ += glm::vec2((*it)->group(View::MIXING)->translation_);
    }
    // compute barycenter (2)
    center_pos_ /= sources_.size();

    // sort the vector of sources in clockwise order around the center pos_
    std::sort(sources_.begin(), sources_.end(), clockwise_centered(center_pos_));

    root_ = new Group;
    center_ = new Symbol(Symbol::CIRCLE_POINT);
    center_->visible_ = false;
    center_->color  = glm::vec4(COLOR_MIXING_GROUP, 0.75f);
    center_->scale_ = glm::vec3(0.6f, 0.6f, 1.f);
    center_->translation_ = glm::vec3(center_pos_, 0.f);
    root_->attach(center_);
    createLineStrip();
}

MixingGroup::~MixingGroup ()
{
    delete center_;
    delete root_;
    if (lines_)
        delete lines_;
}

void MixingGroup::update (float dt)
{
    // active if current source in the group
    auto currentsource = std::find(sources_.begin(), sources_.end(), Mixer::manager().currentSource());
    setActive(currentsource != sources_.end());

    // perform action
    if (update_action_ != ACTION_NONE && updated_source_ != nullptr) {

        if (update_action_ == ACTION_GRAB_ONE ) {

            // update path
            move(updated_source_);

            // compute barycenter (0)
            center_pos_ = glm::vec2(0.f, 0.f);
            for (auto it = sources_.begin(); it != sources_.end(); it++){
                // compute barycenter (1)
                center_pos_ += glm::vec2((*it)->group(View::MIXING)->translation_);
            }
            // compute barycenter (2)
            center_pos_ /= sources_.size();
            center_->translation_ = glm::vec3(center_pos_, 0.f);
        }
        else if (update_action_ == ACTION_GRAB_ALL ) {

            std::vector<glm::vec2> p = lines_->path();
            glm::vec2 displacement = glm::vec2(updated_source_->group(View::MIXING)->translation_);
            displacement -= p[ index_points_[updated_source_] ];

            // compute barycenter (0)
            center_pos_ = glm::vec2(0.f, 0.f);
            auto it = sources_.begin();
            for (; it != sources_.end(); it++){

                // modify all but the already updated source
                if ( *it != updated_source_ && !(*it)->locked() ) {
                    (*it)->group(View::MIXING)->translation_.x += displacement.x;
                    (*it)->group(View::MIXING)->translation_.y += displacement.y;
                    (*it)->touch();
                }

                // update point
                p[ index_points_[*it] ] = glm::vec2((*it)->group(View::MIXING)->translation_);

                // compute barycenter (1)
                center_pos_ += glm::vec2((*it)->group(View::MIXING)->translation_);
            }
            // compute barycenter (2)
            center_pos_ /= sources_.size();
            center_->translation_ = glm::vec3(center_pos_, 0.f);

            // update path
            lines_->changePath(p);
        }
        else if (update_action_ == ACTION_ROTATE_ALL ) {

            std::vector<glm::vec2> p = lines_->path();

            // get angle rotation and distance scaling
            glm::vec2 pos_first = glm::vec2(updated_source_->group(View::MIXING)->translation_) -center_pos_;
            float angle = glm::orientedAngle( glm::normalize(pos_first), glm::vec2(1.f, 0.f) );
            float dist  = glm::length( pos_first );
            glm::vec2 pos_second = glm::vec2(p[ index_points_[updated_source_] ]) -center_pos_;
            angle -= glm::orientedAngle( glm::normalize(pos_second), glm::vec2(1.f, 0.f) );
            dist /= glm::length( pos_second );

            auto it = sources_.begin();
            for (; it != sources_.end(); it++){

                // modify all but the already updated source
                if ( *it != updated_source_  && !(*it)->locked() ) {
                    glm::vec2 vec = glm::vec2((*it)->group(View::MIXING)->translation_) -center_pos_;
                    vec = glm::rotate(vec, -angle) * dist;
                    vec += center_pos_;

                    (*it)->group(View::MIXING)->translation_.x = vec.x;
                    (*it)->group(View::MIXING)->translation_.y = vec.y;
                    (*it)->touch();
                }

                // update point
                p[ index_points_[*it] ] = glm::vec2((*it)->group(View::MIXING)->translation_);
            }
            // update path
            lines_->changePath(p);
        }

        // done
        updated_source_ = nullptr;
    }

}



void MixingGroup::setActive (bool on)
{
    active_ = on;

    // overlays
    lines_->shader()->color.a = active_ ? 0.96f : 0.5f;
    center_->visible_ = update_action_ != ACTION_NONE;
}

void MixingGroup::detach (Source *s)
{
    // find the source
    std::vector<Source *>::iterator its = std::find(sources_.begin(), sources_.end(), s);
    // ok, its in the list !
    if (its != sources_.end()) {
        // erase the source from the list
        sources_.erase(its);
        // clear index, delete lines_, and recreate path and index with remaining sources
        createLineStrip();
    }
}

void MixingGroup::move (Source *s)
{
    // find the source
    std::vector<Source *>::iterator its = std::find(sources_.begin(), sources_.end(), s);
    // ok, its in the list !
    if (its != sources_.end() && lines_) {
        // modify one point in the path
        lines_->editPath(index_points_[s], glm::vec2(s->group(View::MIXING)->translation_));
    }

}

void MixingGroup::createLineStrip()
{
    if (lines_) {
        root_->detach(lines_);
        delete lines_;
    }

    index_points_.clear();

    if (sources_.size() > 1) {
        std::vector<glm::vec2> path;
        auto it = sources_.begin();
        // link sources
        for (; it != sources_.end(); it++){
            index_points_[*it] = path.size();
            path.push_back(glm::vec2((*it)->group(View::MIXING)->translation_));
        }

        // create
        lines_ = new LineLoop(path, 1.5f);
        lines_->shader()->color = glm::vec4(COLOR_MIXING_GROUP, 0.96f);

        root_->attach(lines_);
    }
}
