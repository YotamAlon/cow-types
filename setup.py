from pathlib import Path
from setuptools import setup, Extension

HERE = Path(__file__).parent

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
            depends=[
                "src/cow/cow_utils.h",
                "src/cow/cowdict.h",
                "src/cow/cowlist.h",
                "src/cow/cowset.h",
            ],
            include_dirs=[str(HERE / "src" / "cow")],
            extra_compile_args=["-O2", "-Wall"],
        )
    ],
)
