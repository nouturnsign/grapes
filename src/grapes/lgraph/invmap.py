from typing import Generic, Hashable, Optional, TypeVar, Union

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self

K1 = TypeVar("K1", bound=Hashable)
K2 = TypeVar("K2", bound=Hashable)


class InvertibleMapping(Generic[K1, K2]):
    """Invertible dictionary for internal use."""

    def __init__(
        self: Self,
        original: Optional[dict[K1, K2]] = None,
        inverse: Optional[dict[K1, K2]] = None,
        _linked: bool = False,
    ) -> None:
        if original is None:
            self._original_mapping = {}
        else:
            self._original_mapping = original
        if inverse is None:
            self._inverse_mapping = {}
        else:
            self._inverse_mapping = inverse
        if not _linked:
            self.inverse = self.__class__(
                self._inverse_mapping, self._original_mapping, True
            )
            self.inverse.inverse = self

    def __getitem__(self: Self, key: K1) -> K2:
        return self._original_mapping[key]

    def __setitem__(self: Self, key: K1, value: K2) -> None:
        self._original_mapping[key] = value
        self._inverse_mapping[value] = key

    def __contains__(self: Self, key: Union[K1, K2]) -> bool:
        return (key in self._original_mapping) or (key in self._inverse_mapping)
