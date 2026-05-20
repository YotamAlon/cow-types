from pathlib import Path
from setuptools import setup, Extension

HERE = Path(__file__).parent

setup(
    ext_modules=[
        Extension(
            "cow_types._cow",
            sources=[
                "src/cow_types/cow_utils.c",
                "src/cow_types/cowdict.c",
                "src/cow_types/cowset.c",
                "src/cow_types/cowlist.c",
                "src/cow_types/_cowmodule.c",
            ],
            depends=[
                "src/cow_types/cow_utils.h",
                "src/cow_types/cowdict.h",
                "src/cow_types/cowlist.h",
                "src/cow_types/cowset.h",
            ],
            include_dirs=[str(HERE / "src" / "cow_types")],
            extra_compile_args=["-O2", "-Wall"],
        )
    ],
)
