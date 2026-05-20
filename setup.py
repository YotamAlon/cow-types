from setuptools import setup, Extension

setup(
    ext_modules=[
        Extension(
            "cow._cow",
            sources=[
                "src/cow/cow_utils.c",
                "src/cow/cowdict.c",
                "src/cow/cowset.c",
                "src/cow/cowlist.c",
                "src/cow/_cowmodule.c",
            ],
            include_dirs=["src/cow"],
            extra_compile_args=["-O2", "-Wall"],
        )
    ],
)
