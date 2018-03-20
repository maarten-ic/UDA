from __future__ import (division, unicode_literals, print_function, absolute_import)

from logging import DEBUG, WARNING, INFO, ERROR

from . import c_uda
from ._client import Client, ListType
from ._signal import Signal
from ._dim import Dim
from ._structured import StructuredData
from ._json import SignalEncoder, SignalDecoder

from builtins import dict
from future import standard_library
standard_library.install_aliases()


UDAException = c_uda._c_uda.UDAException

# import the enum values PROP_* from the c_uda library into a Properties class
Properties = type('Properties', (), dict((p, getattr(c_uda, p))
                                         for p in dir(c_uda) if p.startswith('PROP_')))

__all__ = (UDAException, Client, ListType, Signal, Dim, Properties, DEBUG, WARNING, INFO, ERROR)
