//
// Created by matth on 19/12/2018.
//

#include "NovelRenderingService.h"

#include "../lib/SDL2/include/SDL.h"
#include <iostream>

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>

#if OPENGL_VERSION==3
#define NANOVG_GL3_IMPLEMENTATION
#elif OPENGL_VERSION==2
#define NANOVG_GL2_IMPLEMENTATION
#endif
#include "../lib/nanovg/nanovg.h"
#include "../lib/nanovg/nanovg_gl.h"
#include "../lib/nanovg/nanovg_gl_utils.h"
#include "GeoVector.h"
#include "NovelBasicFillRect.h"
#include "NovelImageRect.h"
#include <algorithm>

namespace NovelRT {
    bool NovelRenderingService::sdlInit(const int& displayNumber) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
            std::cerr << "could not initialize sdl2: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_DisplayMode displayData;
        SDL_GetCurrentDisplayMode(displayNumber, &displayData);
        _screenScale = displayData.h / 1080.0f;

        std::cout << _screenScale << std::endl;


        // create window
        _window = SDL_CreateWindow(
                "NovelRTTest", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                displayData.w, displayData.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (_window == nullptr) {
            std::cerr << "could not create window: " << SDL_GetError() << std::endl;

            return false;
        }

        _openGLContext = SDL_GL_CreateContext(_window);
        SDL_GL_MakeCurrent(_window, _openGLContext);
        return true;
    }

    bool NovelRenderingService::nanovgInit() {
#if OPENGL_VERSION==3
        _nanovgContext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#elif OPENGL_VERSION==2
        _nanovgContext = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
        if (_nanovgContext == nullptr) {
            std::cerr << "%llu\n", _nanovgContext;
            std::cerr << "Could not init nanovg.\n";
            return false;
        }

        return true;
    }

    int NovelRenderingService::initialiseRendering(const int& displayNumber) {
        if (!sdlInit(displayNumber)) {
            std::cerr << "Apologies, something went wrong. Reason: SDL could not initialise." << std::endl;
            return 1;
        }

        if (!nanovgInit()) {
            std::cerr << "Apologies, something went wrong. Reason: nanovg could not initialise." << std::endl;
            return 1;
        }


        SDL_GetWindowSize(getWindow(), &_winWidth, &_winHeight);
        _frameBufferWidth = _winWidth;

        _pxRatio = (float) _frameBufferWidth / (float) _winWidth; //TODO: WTF?

        return 0;
    }



    void NovelRenderingService::tearDown() const {
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void NovelRenderingService::renderAllObjects() const {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        nvgBeginFrame(_nanovgContext, _winWidth, _winHeight, _pxRatio);

        for(const auto& value : _renderObjects){
            for(const auto& renderObj : value.second){
                renderObj->drawObject();
            }
        }



        nvgEndFrame(_nanovgContext);
        SDL_GL_SwapWindow(_window);
    }

    void NovelRenderingService::runOnUpdate(NovelSubscriber subscriber) {
        _updateSubscribers.push_back(subscriber);
    }

    NovelImageRect NovelRenderingService::getImageRect(const std::string_view filePath, const GeoVector<float> &startingPosition,
                                                        const float& startingRotation,
                                                        const GeoVector<float>& startingScale, const int& layer,
                                                        const int& orderInLayer) {

        return NovelImageRect(this, _screenScale,startingPosition, filePath, startingRotation, startingScale, layer, orderInLayer);
    }

    void NovelRenderingService::updateRenderingLayerInfo(const int &layer, NovelObject* targetObject, const bool& migrate) {
        if (migrate) {
            auto vec = _renderObjects[targetObject->getLayer()];
            vec.erase(std::remove_if(vec.begin(), vec.end(), [targetObject](const NovelObject* x) {
                auto result = x == targetObject;
                return result;
            }), vec.end());
        }
        auto it = _renderObjects.find(layer);
        if (it == _renderObjects.end()) {
            _renderObjects.insert({layer, std::vector<NovelObject*>()});
        }
        _renderObjects[layer].push_back(targetObject);
        sortLayerRenderOrder(layer);
    }

    void NovelRenderingService::sortLayerRenderOrder(const int& layer) {
        sort(_renderObjects[layer].begin(), _renderObjects[layer].end()); }

    void NovelRenderingService::executeUpdateSubscriptions(const float &deltaTime) const {
        for(const auto& subscriber : _updateSubscribers) {
            subscriber(deltaTime);
        }
    }

    NVGcontext* NovelRenderingService::getNanoVGContext() const {
        return _nanovgContext;
    }

    SDL_Window *NovelRenderingService::getWindow() const {
        return _window;
    }

    void NovelRenderingService::stopRunningOnUpdate(NovelSubscriber subscriber) {
        if(std::find(
                _updateSubscribers.begin(),
                _updateSubscribers.end(),
                subscriber) != _updateSubscribers.end()) {
            _updateSubscribers.erase(std::remove_if(
                    _updateSubscribers.begin(),
                    _updateSubscribers.end(),
                    [subscriber](void (*existingSubscriber)(const float)) {
                        return subscriber == existingSubscriber;
                    }));
        }
        else {
            return;
        }
    }
}