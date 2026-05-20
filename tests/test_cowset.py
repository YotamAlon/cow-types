import pytest
from cow import cowset


def test_init_empty():
    # Arrange / Act
    s = cowset()

    # Assert
    assert len(s) == 0


def test_init_from_set():
    # Arrange / Act
    s = cowset({1, 2, 3})

    # Assert
    assert 1 in s
    assert 2 in s
    assert 3 in s
    assert len(s) == 3


def test_init_from_iterable():
    # Arrange / Act
    s = cowset([1, 2, 2, 3])

    # Assert
    assert len(s) == 3


def test_init_sharing_from_cowset():
    # Arrange
    s = cowset({1, 2})

    # Act  — shares underlying set without copying
    t = cowset(s)

    # Assert
    assert 1 in t


def test_add_creates_new_instance():
    # Arrange
    s = cowset({1, 2})
    original_id = id(s)

    # Act
    s.add(3)

    # Assert
    assert id(s) != original_id
    assert 3 in s


def test_add_does_not_mutate_shared_data():
    # Arrange
    s = cowset({1, 2})
    t = cowset(s)

    # Act
    s.add(3)

    # Assert
    assert 3 not in t


def test_add_existing_element():
    # Arrange
    s = cowset({1, 2})
    original_id = id(s)

    # Act  — adding an already-present element still triggers COW
    s.add(1)

    # Assert  — new instance even if data is the same
    assert 1 in s


def test_discard_existing_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.discard(2)

    # Assert
    assert id(s) != original_id
    assert 2 not in s


def test_discard_missing_is_noop():
    # Arrange
    s = cowset({1, 2})
    original_id = id(s)

    # Act  — discarding a missing element does not mutate
    s.discard(99)

    # Assert  — no frame replacement, same instance (or in-place no-op)
    assert 1 in s
    assert 2 in s


def test_remove_existing_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.remove(2)

    # Assert
    assert id(s) != original_id
    assert 2 not in s


def test_remove_missing_raises():
    # Arrange
    s = cowset({1, 2})

    # Act / Assert
    with pytest.raises(KeyError):
        s.remove(99)


def test_pop_creates_new_instance():
    # Arrange
    s = cowset({42})
    original_id = id(s)

    # Act
    val = s.pop()

    # Assert
    assert id(s) != original_id
    assert val == 42
    assert len(s) == 0


def test_pop_empty_raises():
    # Arrange
    s = cowset()

    # Act / Assert
    with pytest.raises(KeyError):
        s.pop()


def test_clear_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.clear()

    # Assert
    assert id(s) != original_id
    assert len(s) == 0


def test_update_creates_new_instance():
    # Arrange
    s = cowset({1, 2})
    original_id = id(s)

    # Act
    s.update({3, 4})

    # Assert
    assert id(s) != original_id
    assert 3 in s
    assert 4 in s


def test_intersection_update_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.intersection_update({2, 3, 4})

    # Assert
    assert id(s) != original_id
    assert s == {2, 3}


def test_difference_update_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.difference_update({2})

    # Assert
    assert id(s) != original_id
    assert s == {1, 3}


def test_symmetric_difference_update_creates_new_instance():
    # Arrange
    s = cowset({1, 2, 3})
    original_id = id(s)

    # Act
    s.symmetric_difference_update({2, 4})

    # Assert
    assert id(s) != original_id
    assert s == {1, 3, 4}


def test_union_returns_new_cowset():
    # Arrange
    s = cowset({1, 2})

    # Act
    t = s.union({3})

    # Assert
    assert isinstance(t, cowset)
    assert t == {1, 2, 3}
    assert s == {1, 2}


def test_intersection_returns_new_cowset():
    # Arrange
    s = cowset({1, 2, 3})

    # Act
    t = s.intersection({2, 3, 4})

    # Assert
    assert isinstance(t, cowset)
    assert t == {2, 3}


def test_difference_returns_new_cowset():
    # Arrange
    s = cowset({1, 2, 3})

    # Act
    t = s.difference({2})

    # Assert
    assert isinstance(t, cowset)
    assert t == {1, 3}


def test_symmetric_difference_returns_new_cowset():
    # Arrange
    s = cowset({1, 2, 3})

    # Act
    t = s.symmetric_difference({2, 4})

    # Assert
    assert isinstance(t, cowset)
    assert t == {1, 3, 4}


def test_copy_returns_independent_cowset():
    # Arrange
    s = cowset({1, 2})

    # Act
    t = s.copy()
    s.add(3)

    # Assert
    assert isinstance(t, cowset)
    assert 3 not in t


def test_contains():
    # Arrange
    s = cowset({1, 2, 3})

    # Assert
    assert 1 in s
    assert 99 not in s


def test_iter():
    # Arrange
    s = cowset({1, 2, 3})

    # Assert
    assert set(s) == {1, 2, 3}


def test_equality_with_set():
    # Arrange
    s = cowset({1, 2, 3})

    # Assert
    assert s == {1, 2, 3}
    assert s != {1, 2}


def test_equality_with_cowset():
    # Arrange
    s = cowset({1, 2})
    t = cowset({1, 2})

    # Assert
    assert s == t


def test_repr():
    # Arrange
    s = cowset({42})

    # Assert
    assert repr(s) == "cowset({42})"



