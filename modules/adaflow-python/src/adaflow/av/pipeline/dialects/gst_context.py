# Copyright (c) Alibaba, Inc. and its affiliates.
import threading, logging
import gi
from gi.repository import Gst, GLib


class GstContext:
    def __init__(self):
        # SIGINT handle issue:
        # https://github.com/beetbox/audioread/issues/63#issuecomment-390394735
        self._main_loop = GLib.MainLoop.new(None, False)

        self._main_loop_thread = threading.Thread(target=self._main_loop_run)

        self._log = logging.getLogger("pygst.{}".format(self.__class__.__name__))

    def __str__(self) -> str:
        return self.__class__.__name__

    def __repr__(self) -> str:
        return "<{}>".format(self)

    def __enter__(self):
        self.startup()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.shutdown()

    @property
    def log(self) -> logging.Logger:
        return self._log

    def startup(self):
        if self._main_loop_thread.is_alive():
            return

        self._main_loop_thread.start()

    def _main_loop_run(self):
        try:
            self._main_loop.run()
        except Exception:
            pass

    def shutdown(self, timeout: int = 2):
        self.log.debug("%s Quitting main loop ...", self)

        if self._main_loop.is_running():
            self._main_loop.quit()

        self.log.debug("%s Joining main loop thread...", self)
        try:
            if self._main_loop_thread.is_alive():
                self._main_loop_thread.join(timeout=timeout)
        except Exception as err:
            self.log.error("%s.main_loop_thread : %s", self, err)
            pass
