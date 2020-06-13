#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/component_wise.hpp>

#include "Decorations.h"

#include "Visitor.h"
#include "BoundingBoxVisitor.h"
#include "ImageShader.h"
#include "GlmToolkit.h"
#include "Log.h"


Frame::Frame(Type type) : Node(), type_(type), side_(nullptr), top_(nullptr), shadow_(nullptr), square_(nullptr)
{
    color = glm::vec4( 1.f, 1.f, 1.f, 1.f);
    switch (type) {
    case SHARP_LARGE:
        square_ = new LineSquare( 3 );
        shadow_ = new Mesh("mesh/glow.ply", "images/glow.dds");
        break;
    case SHARP_THIN:
        square_ = new LineSquare( 3 );
        break;
    case ROUND_LARGE:
        side_  = new Mesh("mesh/border_large_round.ply");
        top_   = new Mesh("mesh/border_large_top.ply");
        shadow_  = new Mesh("mesh/shadow.ply", "images/shadow.dds");
        break;
    default:
    case ROUND_THIN:
        side_  = new Mesh("mesh/border_round.ply");
        top_   = new Mesh("mesh/border_top.ply");
        shadow_  = new Mesh("mesh/shadow.ply", "images/shadow.dds");
        break;
    case ROUND_SHADOW:
        side_  = new Mesh("mesh/border_round.ply");
        top_   = new Mesh("mesh/border_top.ply");
        shadow_  = new Mesh("mesh/shadow_perspective.ply", "images/shadow_perspective.dds");
        break;
    }

}

Frame::~Frame()
{
    if(side_)  delete side_;
    if(top_)  delete top_;
    if(shadow_)  delete shadow_;
}

void Frame::update( float dt )
{
    Node::update(dt);
    if(top_)
        top_->update(dt);
    if(side_)
        side_->update(dt);
    if(shadow_)
        shadow_->update(dt);
}

void Frame::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() ) {
        if(side_)  side_->init();
        if(top_)   top_->init();
        if(shadow_)  shadow_->init();
        init();
    }

    if ( visible_ ) {

        glm::mat4 ctm = modelview * transform_;

        // shadow (scaled)
        if(shadow_){
            shadow_->shader()->color.a = 0.8f;
            shadow_->draw( ctm, projection);
        }

        // top (scaled)
        if(top_) {
            top_->shader()->color = color;
            top_->draw( ctm, projection);
        }

        // top (scaled)
        if(square_) {
            square_->shader()->color = color;
            square_->draw( ctm, projection);
        }

        if(side_) {

            side_->shader()->color = color;

            // get scale
            glm::vec4 scale = ctm * glm::vec4(1.f, 1.0f, 0.f, 0.f);

            // get rotation
            glm::vec3 rot(0.f);
            glm::vec4 vec = ctm * glm::vec4(1.f, 0.f, 0.f, 0.f);
            rot.z = glm::orientedAngle( glm::vec3(1.f, 0.f, 0.f), glm::normalize(glm::vec3(vec)), glm::vec3(0.f, 0.f, 1.f) );

            if(side_) {

                // left side
                vec = ctm * glm::vec4(1.f, 0.f, 0.f, 1.f);
                side_->draw( GlmToolkit::transform(vec, rot, glm::vec3(scale.y, scale.y, 1.f)), projection );

                // right side
                vec = ctm * glm::vec4(-1.f, 0.f, 0.f, 1.f);
                side_->draw( GlmToolkit::transform(vec, rot, glm::vec3(-scale.y, scale.y, 1.f)), projection );

            }
        }
    }
}


void Frame::accept(Visitor& v)
{
    Node::accept(v);
    v.visit(*this);
}

Handles::Handles(Type type) : Node(), type_(type)
{
    color   = glm::vec4( 1.f, 1.f, 0.f, 1.f);

    if ( type_ == ROTATE ) {
        handle_ = new Mesh("mesh/border_handles_rotation.ply");
    }
    else {
//        handle_ = new LineSquare(color, int ( 2.1f * Rendering::manager().DPIScale()) );
        handle_ = new Mesh("mesh/border_handles_overlay.ply");
    }

}

Handles::~Handles()
{
    if(handle_) delete handle_;
}

void Handles::update( float dt )
{
    Node::update(dt);
    handle_->update(dt);
}

void Handles::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() ) {
        if(handle_) handle_->init();
        init();
    }

    if ( visible_ ) {

        // set color
        handle_->shader()->color = color;

        glm::mat4 ctm;
        glm::vec3 rot(0.f);
        glm::vec4 vec = modelview * glm::vec4(1.f, 0.f, 0.f, 0.f);
        rot.z = glm::orientedAngle( glm::vec3(1.f, 0.f, 0.f), glm::normalize(glm::vec3(vec)), glm::vec3(0.f, 0.f, 1.f) );
        vec = modelview * glm::vec4(0.f, 1.f, 0.f, 1.f);
//        glm::vec3 scale( vec.x > 0.f ? 1.f : -1.f, vec.y > 0.f ? 1.f : -1.f, 1.f);
//        glm::vec3 scale(1.f, 1.f, 1.f);

//        Log::Info(" (0,1) becomes (%f, %f)", scale.x, scale.y);

        if ( type_ == RESIZE ) {

            // 4 corners
            vec = modelview * glm::vec4(1.f, -1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );

            vec = modelview * glm::vec4(1.f, 1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );

            vec = modelview * glm::vec4(-1.f, -1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );

            vec = modelview * glm::vec4(-1.f, 1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );
        }
        else if ( type_ == RESIZE_H ){
            // left and right
            vec = modelview * glm::vec4(1.f, 0.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );

            vec = modelview * glm::vec4(-1.f, 0.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );
        }
        else if ( type_ == RESIZE_V ){
            // top and bottom
            vec = modelview * glm::vec4(0.f, 1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );

            vec = modelview * glm::vec4(0.f, -1.f, 0.f, 1.f);
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));
            handle_->draw( ctm, projection );
        }
        else if ( type_ == ROTATE ){
            // one icon in top right corner
            // 1. Fixed displacement by (0.12,0.12) along the rotation..
            ctm = GlmToolkit::transform(glm::vec4(0.f), rot, glm::vec3(1.f));
            glm::vec4 pos = ctm * glm::vec4(0.12f, 0.12f, 0.f, 1.f);
//            Log::Info(" (0.12,0.12) becomes (%f, %f)", pos.x, pos.y);
            // 2. ..from the top right corner (1,1)
            vec = ( modelview * glm::vec4(1.f, 1.f, 0.f, 1.f) ) + pos;
            ctm = GlmToolkit::transform(vec, rot, glm::vec3(1.f));

// TODO fix problem with negative scale
//            glm::vec4 target = modelview * glm::vec4(1.2f, 1.2f, 0.f, 1.f);

//            vec = modelview * glm::vec4(1.f, 1.f, 0.f, 1.f);
//            glm::vec4 dv = target - vec;

//            Log::Info("dv  (%f, %f)", dv.x, dv.y);
//            float m = dv.x < dv.y ? dv.x : dv.y;
//            Log::Info("min  %f", m);

//            ctm = GlmToolkit::transform( glm::vec3(target), rot, glm::vec3(1.f));

            handle_->draw( ctm, projection );
        }
    }
}


void Handles::accept(Visitor& v)
{
    Node::accept(v);
    v.visit(*this);
}


Icon::Icon(Type style, glm::vec3 pos) : Node()
{
    color   = glm::vec4( 1.f, 1.f, 1.f, 1.f);
    translation_ = pos;

    switch (style) {
    case IMAGE:
        icon_  = new Mesh("mesh/icon_image.ply");
        break;
    case VIDEO:
        icon_  = new Mesh("mesh/icon_video.ply");
        break;
    case SESSION:
        icon_  = new Mesh("mesh/icon_vimix.ply");
        break;
    case CLONE:
        icon_  = new Mesh("mesh/icon_clone.ply");
        break;
    case RENDER:
        icon_  = new Mesh("mesh/icon_render.ply");
        break;
    case EMPTY:
        icon_  = new Mesh("mesh/icon_empty.ply");
        break;
    default:
    case GENERIC:
        icon_  = new Mesh("mesh/point.ply");
        break;
    }

}

Icon::~Icon()
{
    if(icon_)  delete icon_;
}

void Icon::draw(glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() ) {
        if(icon_)  icon_->init();
        init();
    }

    if ( visible_ ) {

        if(icon_) {
            // set color
            icon_->shader()->color = color;

            glm::mat4 ctm = modelview * transform_;
            // correct for aspect ratio
            glm::vec4 vec = ctm * glm::vec4(1.f, 1.0f, 0.f, 0.f);
            ctm *= glm::scale(glm::identity<glm::mat4>(), glm::vec3( vec.y / vec.x, 1.f, 1.f));

            icon_->draw( ctm, projection);
        }
    }
}


void Icon::accept(Visitor& v)
{
    Node::accept(v);
    v.visit(*this);
}


Box::Box()
{
//    color = glm::vec4( 1.f, 1.f, 1.f, 1.f);
    color = glm::vec4( 1.f, 0.f, 0.f, 1.f);
    square_ = new LineSquare( 3 );

}

void Box::draw (glm::mat4 modelview, glm::mat4 projection)
{
    if ( !initialized() ) {
        square_->init();
        init();
    }

    if (visible_) {

        // use a visitor bounding box to calculate extend of all selected nodes
        BoundingBoxVisitor vbox;

        // visit every child of the selection
        for (NodeSet::iterator node = children_.begin();
             node != children_.end(); node++) {
            // reset the transform before
            vbox.setModelview(glm::identity<glm::mat4>());
            (*node)->accept(vbox);
        }

        // get the bounding box
        bbox_ = vbox.bbox();

//        Log::Info("                                       -------- visitor box (%f, %f)-(%f, %f)", bbox_.min().x, bbox_.min().y, bbox_.max().x, bbox_.max().y);

        // set color
        square_->shader()->color = color;

        // compute transformation from bounding box
//        glm::mat4 ctm = modelview * GlmToolkit::transform(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f));
        glm::mat4 ctm = modelview * GlmToolkit::transform(bbox_.center(), glm::vec3(0.f), bbox_.scale());

        // draw bbox
//        square_->draw( modelview, projection);
        square_->draw( ctm, projection);

        // DEBUG
//        visible_=false;
    }

}


