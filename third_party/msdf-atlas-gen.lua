return function()
        includedirs {
            'third_party/msdf-atlas-gen',
            'third_party/msdf-atlas-gen/msdfgen',
            'third_party/msdf-atlas-gen/msdfgen/include',
            'third_party/msdf-atlas-gen/msdfgen/freetype/include',
        }

        defines {
            'MSDFGEN_USE_CPP11',
            '_CRT_SECURE_NO_WARNINGS'
        }

        files {
            'third_party/msdf-atlas-gen/msdfgen/core/*.cpp',
            'third_party/msdf-atlas-gen/msdfgen/ext/*.cpp',
            'third_party/msdf-atlas-gen/msdf-atlas-gen/*.cpp',
            'third_party/msdf-atlas-gen/**.h',
            'third_party/msdf-atlas-gen/**.hpp'
        }

        removefiles {
            'third_party/msdf-atlas-gen/msdf-atlas-gen/main.cpp',
            'third_party/msdf-atlas-gen/msdf-atlas-gen/json-export.cpp',
            'third_party/msdf-atlas-gen/msdf-atlas-gen/csv-export.cpp',
            'third_party/msdf-atlas-gen/msdf-atlas-gen/artery-font-export.cpp',
            'third_party/msdf-atlas-gen/msdfgen/resource.h',
            'third_party/msdf-atlas-gen/resource.h',
            'third_party/msdf-atlas-gen/msdfgen/skia/**',
            'third_party/msdf-atlas-gen/artery-font-format/**'
        }

end