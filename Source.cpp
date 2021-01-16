#include <algorithm>
#include <locale>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Source.h"

#include "defines.h"
#include "FrameBuffer.h"
#include "Decorations.h"
#include "Resource.h"
#include "Session.h"
#include "SearchVisitor.h"
#include "ImageShader.h"
#include "ImageProcessingShader.h"
#include "SystemToolkit.h"
#include "Log.h"
#include "Mixer.h"

Source::Source() : initialized_(false), active_(true), locked_(false), need_update_(true), symbol_(nullptr)
{
    // create unique id
    id_ = GlmToolkit::uniqueId();

    sprintf(initials_, "__");
    name_ = "Source";
    mode_ = Source::UNINITIALIZED;

    // create groups and overlays for each view

    // default rendering node
    groups_[View::RENDERING] = new Group;
    groups_[View::RENDERING]->visible_ = false;

    // default mixing nodes
    groups_[View::MIXING] = new Group;
    groups_[View::MIXING]->visible_ = false;
    groups_[View::MIXING]->scale_ = glm::vec3(0.15f, 0.15f, 1.f);
    groups_[View::MIXING]->translation_ = glm::vec3(DEFAULT_MIXING_TRANSLATION, 0.f);

    frames_[View::MIXING] = new Switch;
    Frame *frame = new Frame(Frame::ROUND, Frame::THIN, Frame::DROP);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_DEFAULT_SOURCE, 0.9f);
    frames_[View::MIXING]->attach(frame);
    frame = new Frame(Frame::ROUND, Frame::LARGE, Frame::DROP);
    frame->translation_.z = 0.01;
    frame->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    frames_[View::MIXING]->attach(frame);
    groups_[View::MIXING]->attach(frames_[View::MIXING]);

    overlays_[View::MIXING] = new Group;
    overlays_[View::MIXING]->translation_.z = 0.1;
    overlays_[View::MIXING]->visible_ = false;
    Symbol *center = new Symbol(Symbol::CIRCLE_POINT, glm::vec3(0.f, 0.f, 0.1f));
    overlays_[View::MIXING]->attach(center);
    groups_[View::MIXING]->attach(overlays_[View::MIXING]);

    // default geometry nodes
    groups_[View::GEOMETRY] = new Group;
    groups_[View::GEOMETRY]->visible_ = false;

    frames_[View::GEOMETRY] = new Switch;
    frame = new Frame(Frame::SHARP, Frame::THIN, Frame::NONE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_DEFAULT_SOURCE, 0.7f);
    frames_[View::GEOMETRY]->attach(frame);
    frame = new Frame(Frame::SHARP, Frame::LARGE, Frame::GLOW);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    frames_[View::GEOMETRY]->attach(frame);
    groups_[View::GEOMETRY]->attach(frames_[View::GEOMETRY]);

    overlays_[View::GEOMETRY] = new Group;
    overlays_[View::GEOMETRY]->translation_.z = 0.15;
    overlays_[View::GEOMETRY]->visible_ = false;
    handles_[View::GEOMETRY][Handles::RESIZE] = new Handles(Handles::RESIZE);
    handles_[View::GEOMETRY][Handles::RESIZE]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::RESIZE]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::RESIZE]);
    handles_[View::GEOMETRY][Handles::RESIZE_H] = new Handles(Handles::RESIZE_H);
    handles_[View::GEOMETRY][Handles::RESIZE_H]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::RESIZE_H]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::RESIZE_H]);
    handles_[View::GEOMETRY][Handles::RESIZE_V] = new Handles(Handles::RESIZE_V);
    handles_[View::GEOMETRY][Handles::RESIZE_V]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::RESIZE_V]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::RESIZE_V]);
    handles_[View::GEOMETRY][Handles::ROTATE] = new Handles(Handles::ROTATE);
    handles_[View::GEOMETRY][Handles::ROTATE]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::ROTATE]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::ROTATE]);
    handles_[View::GEOMETRY][Handles::SCALE] = new Handles(Handles::SCALE);
    handles_[View::GEOMETRY][Handles::SCALE]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::SCALE]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::SCALE]);
    handles_[View::GEOMETRY][Handles::MENU] = new Handles(Handles::MENU);
    handles_[View::GEOMETRY][Handles::MENU]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::MENU]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::MENU]);
    handles_[View::GEOMETRY][Handles::CROP] = new Handles(Handles::CROP);
    handles_[View::GEOMETRY][Handles::CROP]->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    handles_[View::GEOMETRY][Handles::CROP]->translation_.z = 0.1;
    overlays_[View::GEOMETRY]->attach(handles_[View::GEOMETRY][Handles::CROP]);

    frame = new Frame(Frame::SHARP, Frame::THIN, Frame::NONE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 0.7f);
    overlays_[View::GEOMETRY]->attach(frame);
    groups_[View::GEOMETRY]->attach(overlays_[View::GEOMETRY]);

    // default layer nodes
    groups_[View::LAYER] = new Group;
    groups_[View::LAYER]->visible_ = false;
    groups_[View::LAYER]->translation_.z = -1.f;

    frames_[View::LAYER] = new Switch;
    frame = new Frame(Frame::ROUND, Frame::THIN, Frame::PERSPECTIVE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_DEFAULT_SOURCE, 0.8f);    
    frames_[View::LAYER]->attach(frame);
    frame = new Frame(Frame::ROUND, Frame::LARGE, Frame::PERSPECTIVE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_HIGHLIGHT_SOURCE, 1.f);
    frames_[View::LAYER]->attach(frame);
    groups_[View::LAYER]->attach(frames_[View::LAYER]);

    overlays_[View::LAYER] = new Group;
    overlays_[View::LAYER]->translation_.z = 0.15;
    overlays_[View::LAYER]->visible_ = false;
    groups_[View::LAYER]->attach(overlays_[View::LAYER]);

    // default appearance node
    groups_[View::APPEARANCE] = new Group;
    groups_[View::APPEARANCE]->visible_ = false;

    frames_[View::APPEARANCE] = new Switch;
    frame = new Frame(Frame::SHARP, Frame::THIN, Frame::NONE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 0.7f);
    frames_[View::APPEARANCE]->attach(frame);
    frame = new Frame(Frame::SHARP, Frame::LARGE, Frame::NONE);
    frame->translation_.z = 0.1;
    frame->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    frames_[View::APPEARANCE]->attach(frame);
    groups_[View::APPEARANCE]->attach(frames_[View::APPEARANCE]);

    overlays_[View::APPEARANCE] = new Group;
    overlays_[View::APPEARANCE]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->visible_ = false;
    handles_[View::APPEARANCE][Handles::RESIZE] = new Handles(Handles::RESIZE);
    handles_[View::APPEARANCE][Handles::RESIZE]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::RESIZE]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::RESIZE]);
    handles_[View::APPEARANCE][Handles::RESIZE_H] = new Handles(Handles::RESIZE_H);
    handles_[View::APPEARANCE][Handles::RESIZE_H]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::RESIZE_H]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::RESIZE_H]);
    handles_[View::APPEARANCE][Handles::RESIZE_V] = new Handles(Handles::RESIZE_V);
    handles_[View::APPEARANCE][Handles::RESIZE_V]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::RESIZE_V]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::RESIZE_V]);
    handles_[View::APPEARANCE][Handles::ROTATE] = new Handles(Handles::ROTATE);
    handles_[View::APPEARANCE][Handles::ROTATE]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::ROTATE]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::ROTATE]);
    handles_[View::APPEARANCE][Handles::SCALE] = new Handles(Handles::SCALE);
    handles_[View::APPEARANCE][Handles::SCALE]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::SCALE]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::SCALE]);
    handles_[View::APPEARANCE][Handles::MENU] = new Handles(Handles::MENU);
    handles_[View::APPEARANCE][Handles::MENU]->color = glm::vec4( COLOR_APPEARANCE_SOURCE, 1.f);
    handles_[View::APPEARANCE][Handles::MENU]->translation_.z = 0.1;
    overlays_[View::APPEARANCE]->attach(handles_[View::APPEARANCE][Handles::MENU]);
    groups_[View::APPEARANCE]->attach(overlays_[View::APPEARANCE]);

    // empty transition node
    groups_[View::TRANSITION] = new Group;

    //
    // shared locker symbol
    //
    locker_ = new Symbol(Symbol::LOCK, glm::vec3(0.8f, -0.8f, 0.01f));
    locker_->color = glm::vec4(1.f, 1.f, 1.f, 0.6f);

    // add semi transparent icon statically to mixing and layer views
    Group *lockgroup = new Group;
    lockgroup->translation_.z = 0.1;
    lockgroup->attach( locker_ );
    groups_[View::LAYER]->attach(lockgroup);
    groups_[View::MIXING]->attach(lockgroup);

    // add semi transparent icon dynamically with overlay
    overlays_[View::LAYER]->attach( locker_ );
    overlays_[View::MIXING]->attach( locker_ );

    // create objects
    stored_status_  = new Group;

    // simple image shader (with texturing) for blending
    blendingshader_ = new ImageShader;
    // mask produced by dedicated shader
    maskshader_ = new MaskShader;
    masksurface_ = new Surface(maskshader_);

    // filtered image shader (with texturing and processing) for rendering
    processingshader_   = new ImageProcessingShader;
    // default rendering with image processing enabled
    renderingshader_ = (Shader *) processingshader_;

    // for drawing in mixing view
    mixingshader_ = new ImageShader;
    mixingshader_->stipple = 1.0;

    // create media surface:
    // - textured with original texture from media player
    // - crop & repeat UV can be managed here
    // - additional custom shader can be associated
    texturesurface_ = new Surface(renderingshader_);

    // will be created at init
    renderbuffer_   = nullptr;
    rendersurface_  = nullptr;
    mixingsurface_  = nullptr;
    maskbuffer_     = nullptr;
    maskimage_      = nullptr;
    mask_need_update_ = false;
}


Source::~Source()
{
    // inform clones that they lost their origin
    for (auto it = clones_.begin(); it != clones_.end(); it++)
        (*it)->detach();
    clones_.clear();

    // delete objects
    delete stored_status_;
    if (renderbuffer_)
        delete renderbuffer_;
    if (maskbuffer_)
        delete maskbuffer_;
    if (maskimage_)
        delete maskimage_;

    // all groups and their children are deleted in the scene
    // this includes rendersurface_, overlays, blendingshader_ and rendershader_
    delete groups_[View::RENDERING];
    delete groups_[View::MIXING];
    delete groups_[View::GEOMETRY];
    delete groups_[View::LAYER];
    delete groups_[View::APPEARANCE];
    delete groups_[View::TRANSITION];

    groups_.clear();
    frames_.clear();
    overlays_.clear();

    // don't forget that the processing shader
    // could be created but not used
    if ( renderingshader_ != processingshader_ )
        delete processingshader_;

    delete texturesurface_;
}

void Source::setName (const std::string &name)
{
    name_ = SystemToolkit::transliterate(name);

    initials_[0] = std::toupper( name_.front(), std::locale("C") );
    initials_[1] = std::toupper( name_.back(), std::locale("C") );
}

void Source::accept(Visitor& v)
{
    v.visit(*this);
}

Source::Mode Source::mode() const
{
    return mode_;
}

void Source::setMode(Source::Mode m)
{
    // make visible on first time
    if ( mode_ == Source::UNINITIALIZED ) {
        for (auto g = groups_.begin(); g != groups_.end(); g++)
            (*g).second->visible_ = true;
    }

    // choose frame 0 if visible, 1 if selected
    uint index_frame = m == Source::VISIBLE ? 0 : 1;
    for (auto f = frames_.begin(); f != frames_.end(); f++)
        (*f).second->setActive(index_frame);

    // show overlay if current
    bool current = m >= Source::CURRENT;
    for (auto o = overlays_.begin(); o != overlays_.end(); o++)
        (*o).second->visible_ = current;

    // show in appearance view if current
    groups_[View::APPEARANCE]->visible_ = m > Source::VISIBLE;

    mode_ = m;
}


void Source::setImageProcessingEnabled (bool on)
{
    // avoid repeating
    if ( on == imageProcessingEnabled() )
        return;

    // set pointer
    if (on) {
        // set the current rendering shader to be the
        // (previously prepared) processing shader
        renderingshader_ = (Shader *) processingshader_;
    }
    else {
        // clone the current Image processing shader
        //  (because the one currently attached to the source
        //   will be deleted in replaceRenderngShader().)
        ImageProcessingShader *tmp = new ImageProcessingShader(*processingshader_);
        // loose reference to current processing shader (to delete)
        // and keep reference to the newly created one
        // and keep it for later
        processingshader_ = tmp;
        // set the current rendering shader to a simple one
        renderingshader_ = (Shader *) new ImageShader;
    }

    // apply to nodes in subclasses
    // this calls replaceShader() on the Primitive and
    // will delete the previously attached shader
    texturesurface_->replaceShader(renderingshader_);
}

bool Source::imageProcessingEnabled()
{
    return ( renderingshader_ == processingshader_ );
}

void Source::render()
{
    if (!initialized_)
        init();
    else {
        // render the view into frame buffer
        renderbuffer_->begin();
        texturesurface_->draw(glm::identity<glm::mat4>(), renderbuffer_->projection());
        renderbuffer_->end();
    }
}


void Source::attach(FrameBuffer *renderbuffer)
{
    renderbuffer_ = renderbuffer;

    // if a symbol is available, add it to overlay
    if (symbol_) {
        overlays_[View::MIXING]->attach( symbol_ );
        overlays_[View::LAYER]->attach( symbol_ );
    }

    // create the surfaces to draw the frame buffer in the views
    rendersurface_ = new FrameBufferSurface(renderbuffer_, blendingshader_);
    groups_[View::RENDERING]->attach(rendersurface_);
    groups_[View::GEOMETRY]->attach(rendersurface_);

    // for mixing and layer views, add another surface to overlay
    // (stippled view on top with transparency)
    mixingsurface_ = new FrameBufferSurface(renderbuffer_, mixingshader_);
    groups_[View::MIXING]->attach(mixingsurface_);
    groups_[View::LAYER]->attach(mixingsurface_);

    // for views showing a scaled mixing surface, a dedicated transparent surface allows grabbing
    Surface *surfacetmp = new Surface();
    surfacetmp->setTextureIndex(Resource::getTextureTransparent());
    groups_[View::APPEARANCE]->attach(surfacetmp);
    groups_[View::MIXING]->attach(surfacetmp);
    groups_[View::LAYER]->attach(surfacetmp);

    // Transition group node is optionnal
    if (groups_[View::TRANSITION]->numChildren() > 0)
        groups_[View::TRANSITION]->attach(mixingsurface_);

    // scale all icon nodes to match aspect ratio
    for (int v = View::MIXING; v < View::INVALID; v++) {
        NodeSet::iterator node;
        for (node = groups_[(View::Mode) v]->begin();
             node != groups_[(View::Mode) v]->end(); node++) {
            (*node)->scale_.x = renderbuffer_->aspectRatio();
        }
    }

    // hack to place the symbols in the corner independently of aspect ratio
    symbol_->translation_.x += 0.1f * (renderbuffer_->aspectRatio()-1.f);
    locker_->translation_.x += 0.1f * (renderbuffer_->aspectRatio()-1.f);

    // (re) create the masking buffer
    if (maskbuffer_)
        delete maskbuffer_;
    maskbuffer_ = new FrameBuffer( glm::vec3(0.5) * renderbuffer->resolution() );

    // make the source visible
    if ( mode_ == UNINITIALIZED )
        setMode(VISIBLE);

    // request update
    need_update_ = true;
}


void Source::setActive (bool on)
{
    active_ = on;

    // do not disactivate if a clone depends on it
    for(auto clone = clones_.begin(); clone != clones_.end(); clone++) {
        if ( (*clone)->active() )
            active_ = true;
    }

    // an inactive source is visible only in the MIXING view
    groups_[View::RENDERING]->visible_ = active_;
    groups_[View::GEOMETRY]->visible_ = active_;
    groups_[View::LAYER]->visible_ = active_;

}

void Source::setLocked (bool on)
{
    locked_ = on;

    // the lock icon is visible when locked
    locker_->visible_ = on;

    // a locked source is not visible in the GEOMETRY view (that's the whole point of it!)
    groups_[View::GEOMETRY]->visible_ = !locked_;

}


// Transfer functions from coordinates to alpha (1 - transparency)
float linear_(float x, float y) {
    return 1.f - CLAMP( sqrt( ( x * x ) + ( y * y ) ), 0.f, 1.f );
}

float quad_(float x, float y) {
    return 1.f - CLAMP( ( x * x ) + ( y * y ), 0.f, 1.f );
}

float sin_quad(float x, float y) {
    return 0.5f + 0.5f * cos( M_PI * CLAMP( ( ( x * x ) + ( y * y ) ), 0.f, 1.f ) );
}

void Source::update(float dt)
{
    // keep delta-t
    dt_ = dt;

    // update nodes if needed
    if (renderbuffer_ && mixingsurface_ && maskbuffer_ && need_update_)
    {
//        Log::Info("UPDATE %s %f", initials_, dt);

        // ADJUST alpha based on MIXING node
        // read position of the mixing node and interpret this as transparency of render output
        glm::vec2 dist = glm::vec2(groups_[View::MIXING]->translation_);
        // use the prefered transfer function
        blendingshader_->color = glm::vec4(1.0, 1.0, 1.0, sin_quad( dist.x, dist.y ));
        mixingshader_->color = blendingshader_->color;

        // CHANGE update status based on limbo
        bool a = glm::length(dist) < MIXING_LIMBO_SCALE;
        setActive( a );
        // adjust scale of mixing icon : smaller if not active
        groups_[View::MIXING]->scale_ = glm::vec3(MIXING_ICON_SCALE) - ( a ? glm::vec3(0.f, 0.f, 0.f) : glm::vec3(0.03f, 0.03f, 0.f) );

        // MODIFY geometry based on GEOMETRY node
        groups_[View::RENDERING]->translation_ = groups_[View::GEOMETRY]->translation_;
        groups_[View::RENDERING]->rotation_ = groups_[View::GEOMETRY]->rotation_;
        glm::vec3 s = groups_[View::GEOMETRY]->scale_;
        // avoid any null scale
        s.x = CLAMP_SCALE(s.x);
        s.y = CLAMP_SCALE(s.y);
        s.z = 1.f;
        groups_[View::GEOMETRY]->scale_ = s;
        groups_[View::RENDERING]->scale_ = s;

        // MODIFY CROP projection based on GEOMETRY crop
        renderbuffer_->setProjectionArea( glm::vec2(groups_[View::GEOMETRY]->crop_) );

        // Mixing and layer icons scaled based on GEOMETRY crop
        mixingsurface_->scale_ = groups_[View::GEOMETRY]->crop_;
        mixingsurface_->scale_.x *= renderbuffer_->aspectRatio();
        mixingsurface_->update(dt_);

        // Layers icons are displayed in Perspective (diagonal)
        groups_[View::LAYER]->translation_.y = groups_[View::LAYER]->translation_.x / LAYER_PERSPECTIVE;

        // CHANGE lock based on range of layers stage
        bool l = (groups_[View::LAYER]->translation_.x < -FOREGROUND_DEPTH)
                || (groups_[View::LAYER]->translation_.x > -BACKGROUND_DEPTH);
        setLocked( l );

        // adjust position of layer icon: step up when on stage
        if (groups_[View::LAYER]->translation_.x < -FOREGROUND_DEPTH)
            groups_[View::LAYER]->translation_.y -= 0.3f;
        else if (groups_[View::LAYER]->translation_.x < -BACKGROUND_DEPTH)
            groups_[View::LAYER]->translation_.y -= 0.15f;

        // MODIFY depth based on LAYER node
        groups_[View::MIXING]->translation_.z = groups_[View::LAYER]->translation_.z;
        groups_[View::GEOMETRY]->translation_.z = groups_[View::LAYER]->translation_.z;
        groups_[View::RENDERING]->translation_.z = groups_[View::LAYER]->translation_.z;

        // MODIFY texture projection based on APPEARANCE node
        // UV to node coordinates
        static glm::mat4 UVtoScene = GlmToolkit::transform(glm::vec3(1.f, -1.f, 0.f),
                                                           glm::vec3(0.f, 0.f, 0.f),
                                                           glm::vec3(-2.f, 2.f, 1.f));
        // Aspect Ratio correction transform : coordinates of Appearance Frame are scaled by render buffer width
        glm::mat4 Ar = glm::scale(glm::identity<glm::mat4>(), glm::vec3(renderbuffer_->aspectRatio(), 1.f, 1.f) );
        // Translation : same as Appearance Frame (modified by Ar)
        glm::mat4 Tra = glm::translate(glm::identity<glm::mat4>(), groups_[View::APPEARANCE]->translation_);
        // Scaling : inverse scaling (larger UV when smaller Appearance Frame)
        glm::mat4 Sca = glm::scale(glm::identity<glm::mat4>(), glm::vec3(groups_[View::APPEARANCE]->scale_.x,groups_[View::APPEARANCE]->scale_.y, 1.f));
        // Rotation : same angle than Appearance Frame, inverted axis
        glm::mat4 Rot = glm::rotate(glm::identity<glm::mat4>(), groups_[View::APPEARANCE]->rotation_.z, glm::vec3(0.f, 0.f, -1.f) );
        // Combine transformations (non transitive) in this order:
        // 1. switch to Scene coordinate system
        // 2. Apply the aspect ratio correction
        // 3. Apply the translation
        // 4. Apply the rotation (centered after translation)
        // 5. Revert aspect ration correction
        // 6. Apply the Scaling (independent of aspect ratio)
        // 7. switch back to UV coordinate system
        texturesurface_->shader()->iTransform = glm::inverse(UVtoScene) * glm::inverse(Sca) * glm::inverse(Ar) * Rot * Tra * Ar * UVtoScene;

        // if a mask image was given to be updated
        if (mask_need_update_) {
            // fill the mask buffer (once)
            if (maskbuffer_->fill(maskimage_) )
                mask_need_update_ = false;
        }
        // otherwise, render the mask buffer
        else
        {
            // draw mask in mask frame buffer
            maskbuffer_->begin(false);
            // loopback maskbuffer texture for painting
            masksurface_->setTextureIndex(maskbuffer_->texture());
            // fill surface with mask texture
            masksurface_->draw(glm::identity<glm::mat4>(), maskbuffer_->projection());
            maskbuffer_->end();
        }

        // set the rendered mask as mask for blending
        blendingshader_->mask_texture = maskbuffer_->texture();

        // do not update next frame
        need_update_ = false;
    }

}

FrameBuffer *Source::frame() const
{
    if (initialized_ && renderbuffer_)
    {
        return renderbuffer_;
    }
    else {
        static FrameBuffer *black = new FrameBuffer(640,480);
        return black;
    }
}

bool Source::contains(Node *node) const
{
    if ( node == nullptr )
        return false;

    hasNode tester(node);
    return tester(this);
}


void Source::storeMask(FrameBufferImage *img)
{
    // free the output mask storage
    if (maskimage_ != nullptr) {
        delete maskimage_;
        maskimage_ = nullptr;
    }

    // if no image is provided
    if (img == nullptr) {
        // if ready
        if (maskbuffer_!=nullptr) {
            // get & store image from mask buffer
            maskimage_ = maskbuffer_->image();
        }
    }
    else
        // store the given image
        maskimage_ = img;

    // maskimage_ can now be accessed with Source::getStoredMask
}

void Source::setMask(FrameBufferImage *img)
{
    // if a valid image is given
    if (img != nullptr && img->width>0 && img->height>0) {

        // remember this new image as the current mask
        // NB: will be freed when replaced
        storeMask(img);

        // ask Source::update to use it at next update for filling mask buffer
        mask_need_update_ = true;

        // ask to update the source
        touch();
    }
    else
        mask_need_update_ = false;
}


bool Source::hasNode::operator()(const Source* elem) const
{
    if (_n && elem)
    {
        // quick case (most frequent and easy to answer)
        if (elem->rendersurface_ == _n)
            return true;

        // general case: traverse tree of all Groups recursively using a SearchVisitor
        SearchVisitor sv(_n);
        // search in groups for all views
        for (auto g = elem->groups_.begin(); g != elem->groups_.end(); g++) {
            (*g).second->accept(sv);
            if (sv.found())
                return true;
        }
        // search in overlays for all views
        for (auto g = elem->overlays_.begin(); g != elem->overlays_.end(); g++) {
            (*g).second->accept(sv);
            if (sv.found())
                return true;
        }
    }

    return false;
}

CloneSource *Source::clone()
{
    CloneSource *s = new CloneSource(this);

    clones_.push_back(s);

    return s;
}


CloneSource::CloneSource(Source *origin) : Source(), origin_(origin)
{
    // set symbol
    symbol_ = new Symbol(Symbol::CLONE, glm::vec3(0.8f, 0.8f, 0.01f));
}

CloneSource::~CloneSource()
{
    if (origin_)
        origin_->clones_.remove(this);
}

CloneSource *CloneSource::clone()
{
    // do not clone a clone : clone the original instead
    if (origin_)
        return origin_->clone();
    else
        return nullptr;
}

void CloneSource::init()
{
    if (origin_ && origin_->ready()) {

        // get the texture index from framebuffer of view, apply it to the surface
        texturesurface_->setTextureIndex( origin_->texture() );

        // create Frame buffer matching size of session
        FrameBuffer *renderbuffer = new FrameBuffer( origin_->frame()->resolution(), true);

        // set the renderbuffer of the source and attach rendering nodes
        attach(renderbuffer);

        // deep update to reorder
        View::need_deep_update_++;

        // done init
        initialized_ = true;
        Log::Info("Source %s cloning source %s.", name().c_str(), origin_->name().c_str() );
    }
}

void CloneSource::setActive (bool on)
{
    active_ = on;

    groups_[View::RENDERING]->visible_ = active_;
    groups_[View::GEOMETRY]->visible_ = active_;
    groups_[View::LAYER]->visible_ = active_;

    if (initialized_ && origin_ != nullptr)
        origin_->touch();
}


uint CloneSource::texture() const
{
    if (initialized_ && origin_ != nullptr)
        return origin_->texture();
    else
        return Resource::getTextureBlack();
}

void CloneSource::accept(Visitor& v)
{
    Source::accept(v);
    if (!failed())
        v.visit(*this);
}

