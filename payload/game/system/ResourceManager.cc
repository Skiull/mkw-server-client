#include "ResourceManager.hh"

#include "game/system/RootScene.hh"

#include <sp/storage/DecompLoader.hh>
#include <sp/storage/Storage.hh>

extern "C" {
#include <revolution/arc.h>
#include <revolution/os.h>
}

#include <cstdio>
#include <cstring>

namespace System {

void ResourceManager::CourseCache::init() {}

void ResourceManager::CourseCache::load(u32 /* courseId */) {}

ResourceManager *ResourceManager::Instance() {
    return s_instance;
}

ResourceManager *ResourceManager::CreateInstance() {
    s_instance = new ResourceManager;
    assert(s_instance);

    s_instance->m_globe = nullptr;

    return s_instance;
}

void ResourceManager::initGlobeHeap() {
    if (m_globeHeap) {
        return;
    }

    // Allocation failure is not fatal: loadGlobe then hands back no model (see below).
    auto *heap = RootScene::Instance()->m_heapCollection.mem2;
    size_t size = OSRoundUp32B(0x107d080 + 0x20400);
    void *buffer = new (heap, -0x20) u8[size];
    if (!buffer) {
        SP_LOG("ResourceManager: failed to allocate the globe heap (0x%x bytes)", size);
        return;
    }
    m_globeHeap = EGG::ExpHeap::Create(buffer, size, 1);
    if (!m_globeHeap) {
        heap->free(buffer);
    }
}

void ResourceManager::deinitGlobeHeap() {
    if (m_globeHeap) {
        m_globe = nullptr;
        m_globeHeap->destroy();
        m_globeHeap = nullptr;
        m_globeLoadingIsBusy = false;
    }
}

void ResourceManager::OnCreateScene(SceneId sceneId) {
    if (!s_instance) {
        return;
    }

    switch (sceneId) {
    case SceneId::Menu:
    case SceneId::Race:
        s_instance->deinitGlobeHeap();
        break;
    case SceneId::Globe:
        s_instance->initGlobeHeap();
        break;
    default:
        break;
    }
}

void ResourceManager::loadGlobe(u8 **dst) {
    if (m_globe) {
        *dst = m_globe;
        m_globeLoadingIsBusy = false;
        return;
    }

    *dst = nullptr;
    auto *heap = m_globeHeap;
    if (!heap) {
        m_globeLoadingIsBusy = false;
        return;
    }

    const char *path = "/contents/globe.arc";
    auto file = SP::Storage::OpenRO(path);
    if (!file) {
        m_globeLoadingIsBusy = false;
        return;
    }

    alignas(0x20) ARCHeader header;
    if (!file->read(&header, sizeof(header), 0)) {
        m_globeLoadingIsBusy = false;
        return;
    }

    void *metadata = heap->alloc(header.fileStart, 0x20);
    ARCHandle handle;
    ARCFileInfo info;
    if (metadata && file->read(metadata, header.fileStart, 0) &&
            ARCInitHandle(metadata, &handle) && ARCOpen(&handle, "/earth.brres.LZ", &info)) {
        u32 srcSize = AlignUp<u32>(ARCGetLength(&info), 0x20);
        u32 srcOffset = ARCGetStartOffset(&info);
        ARCClose(&info);

        size_t dstSize;
        if (SP::Storage::DecompLoader::LoadRO(path, srcSize, srcOffset, dst, &dstSize, heap)) {
            m_globe = *dst;
        } else {
            *dst = nullptr;
        }
    }
    if (metadata) {
        heap->free(metadata);
    }

    m_globeLoadingIsBusy = false;
}

void ResourceManager::LoadGlobeTask(void *arg) {
    assert(s_instance);
    s_instance->loadGlobe(reinterpret_cast<u8 **>(arg));
}

DvdArchive *ResourceManager::getMenuArchive(size_t idx) {
    MultiDvdArchive *archive = m_archives[static_cast<size_t>(ResourceType::Menu)];
    u16 loadedCount = 0;
    for (u16 i = 0; i < archive->count(); i++) {
        DvdArchive &candidate = archive->archive(i);
        if (candidate.state() == DvdArchive::State::Mounted) {
            if (loadedCount == idx) {
                return &candidate;
            }
            loadedCount++;
        }
    }
    return nullptr;
}

u16 ResourceManager::getMenuArchiveCount() const {
    MultiDvdArchive *archive = m_archives[static_cast<size_t>(ResourceType::Menu)];
    u16 loadedCount = 0;
    for (u16 i = 0; i < archive->count(); i++) {
        if (archive->archive(i).state() == DvdArchive::State::Mounted) {
            loadedCount++;
        }
    }
    return loadedCount;
}

} // namespace System
