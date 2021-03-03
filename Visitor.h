#ifndef VISITOR_H
#define VISITOR_H

#include <string>

// Forward declare different kind of Node
class Node;
class Group;
class Switch;
class Primitive;
class Scene;
class Surface;
class ImageSurface;
class MediaSurface;
class FrameBufferSurface;
class LineStrip;
class LineSquare;
class LineCircle;
class Mesh;
class Frame;
class Handles;
class Symbol;
class Disk;
class Stream;
class MediaPlayer;
class Shader;
class ImageShader;
class MaskShader;
class ImageProcessingShader;
class Source;
class MediaSource;
class PatternSource;
class DeviceSource;
class GenericStreamSource;
class SessionFileSource;
class SessionGroupSource;
class RenderSource;
class CloneSource;
class NetworkSource;

// Declares the interface for the visitors
class Visitor {

public:
    // Need to declare overloads for basic kind of Nodes to visit
    virtual void visit (Scene&) = 0;
    virtual void visit (Node&) = 0;
    virtual void visit (Primitive&) = 0;
    virtual void visit (Group&) = 0;
    virtual void visit (Switch&) = 0;

    // not mandatory for all others
    virtual void visit (Surface&) {}
    virtual void visit (ImageSurface&) {}
    virtual void visit (MediaSurface&) {}
    virtual void visit (FrameBufferSurface&) {}
    virtual void visit (LineStrip&)  {}
    virtual void visit (LineSquare&) {}
    virtual void visit (Mesh&) {}
    virtual void visit (Frame&) {}
    virtual void visit (Handles&) {}
    virtual void visit (Symbol&) {}
    virtual void visit (Disk&) {}
    virtual void visit (Stream&) {}
    virtual void visit (MediaPlayer&) {}
    virtual void visit (Shader&) {}
    virtual void visit (ImageShader&) {}
    virtual void visit (MaskShader&) {}
    virtual void visit (ImageProcessingShader&) {}

    // utility
    virtual void visit (Source&) {}
    virtual void visit (MediaSource&) {}
    virtual void visit (NetworkSource&) {}
    virtual void visit (GenericStreamSource&) {}
    virtual void visit (DeviceSource&) {}
    virtual void visit (PatternSource&) {}
    virtual void visit (SessionFileSource&) {}
    virtual void visit (SessionGroupSource&) {}
    virtual void visit (RenderSource&) {}
    virtual void visit (CloneSource&) {}

};


#endif // VISITOR_H
