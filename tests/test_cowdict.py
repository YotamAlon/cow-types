import pytest
from cow_types import cowdict


def test_init_empty():
    # Arrange / Act
    d = cowdict()

    # Assert
    assert len(d) == 0


def test_init_from_dict():
    # Arrange / Act
    d = cowdict({"a": 1, "b": 2})

    # Assert
    assert d["a"] == 1
    assert d["b"] == 2
    assert len(d) == 2


def test_init_from_kwargs():
    # Arrange / Act
    d = cowdict(a=1, b=2)

    # Assert
    assert d["a"] == 1
    assert d["b"] == 2


def test_init_sharing_from_cowdict():
    # Arrange
    d = cowdict({"x": 1})

    # Act  — shares underlying dict without copying
    e = cowdict(d)

    # Assert
    assert e["x"] == 1


def test_setitem_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1})
    original_id = id(d)

    # Act
    d["b"] = 2

    # Assert
    assert id(d) != original_id
    assert d["b"] == 2
    assert d["a"] == 1


def test_setitem_does_not_mutate_shared_data():
    # Arrange
    d = cowdict({"a": 1})
    e = cowdict(d)

    # Act
    d["b"] = 2

    # Assert  — e still references the old data
    assert "b" not in e
    assert e["a"] == 1


def test_delitem_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1, "b": 2})
    original_id = id(d)

    # Act
    del d["a"]

    # Assert
    assert id(d) != original_id
    assert "a" not in d
    assert d["b"] == 2


def test_delitem_missing_key_raises():
    # Arrange
    d = cowdict({"a": 1})

    # Act / Assert
    with pytest.raises(KeyError):
        del d["z"]


def test_contains():
    # Arrange
    d = cowdict({"a": 1})

    # Assert
    assert "a" in d
    assert "b" not in d


def test_iter():
    # Arrange
    d = cowdict({"a": 1, "b": 2})

    # Act
    keys = list(d)

    # Assert
    assert set(keys) == {"a", "b"}


def test_get_existing_key():
    # Arrange
    d = cowdict({"a": 1})

    # Assert
    assert d.get("a") == 1


def test_get_missing_key_default():
    # Arrange
    d = cowdict({"a": 1})

    # Assert
    assert d.get("z") is None
    assert d.get("z", 99) == 99


def test_keys_values_items():
    # Arrange
    d = cowdict({"a": 1, "b": 2})

    # Assert
    assert set(d.keys()) == {"a", "b"}
    assert set(d.values()) == {1, 2}
    assert set(d.items()) == {("a", 1), ("b", 2)}


def test_copy_returns_independent_cowdict():
    # Arrange
    d = cowdict({"a": 1})

    # Act
    e = d.copy()
    d["b"] = 2

    # Assert
    assert isinstance(e, cowdict)
    assert "b" not in e


def test_update_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1})
    original_id = id(d)

    # Act
    d.update({"b": 2})

    # Assert
    assert id(d) != original_id
    assert d["b"] == 2


def test_update_with_kwargs():
    # Arrange
    d = cowdict({"a": 1})

    # Act
    d.update(b=2, c=3)

    # Assert
    assert d["b"] == 2
    assert d["c"] == 3


def test_pop_returns_value_and_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1, "b": 2})
    original_id = id(d)

    # Act
    val = d.pop("a")

    # Assert
    assert val == 1
    assert id(d) != original_id
    assert "a" not in d


def test_pop_missing_raises():
    # Arrange
    d = cowdict({"a": 1})

    # Act / Assert
    with pytest.raises(KeyError):
        d.pop("z")


def test_pop_missing_with_default():
    # Arrange
    d = cowdict({"a": 1})

    # Act
    val = d.pop("z", 99)

    # Assert
    assert val == 99


def test_popitem_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1})
    original_id = id(d)

    # Act
    item = d.popitem()

    # Assert
    assert id(d) != original_id
    assert item == ("a", 1)
    assert len(d) == 0


def test_clear_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1, "b": 2})
    original_id = id(d)

    # Act
    d.clear()

    # Assert
    assert id(d) != original_id
    assert len(d) == 0


def test_setdefault_existing_key():
    # Arrange
    d = cowdict({"a": 1})
    original_id = id(d)

    # Act
    val = d.setdefault("a", 99)

    # Assert
    assert val == 1
    assert id(d) == original_id


def test_setdefault_new_key_creates_new_instance():
    # Arrange
    d = cowdict({"a": 1})
    original_id = id(d)

    # Act
    val = d.setdefault("b", 2)

    # Assert
    assert val == 2
    assert id(d) != original_id
    assert d["b"] == 2


def test_equality_with_dict():
    # Arrange
    d = cowdict({"a": 1})

    # Assert
    assert d == {"a": 1}
    assert d != {"a": 2}


def test_equality_with_cowdict():
    # Arrange
    d = cowdict({"a": 1})
    e = cowdict({"a": 1})

    # Assert
    assert d == e


def test_repr():
    # Arrange
    d = cowdict({"a": 1})

    # Assert
    assert repr(d) == "cowdict({'a': 1})"


def test_multiple_mutations_each_create_new_instance():
    # Arrange
    d = cowdict({"a": 1})
    id1 = id(d)

    # Act
    d["b"] = 2
    id2 = id(d)
    d["c"] = 3
    id3 = id(d)

    # Assert
    assert id1 != id2
    assert id2 != id3
    assert d["a"] == 1
    assert d["b"] == 2
    assert d["c"] == 3
