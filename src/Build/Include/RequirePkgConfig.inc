pcver=$(shell pkg-config --version 2>/dev/null)

ifeq ($(pcver),)
err = $(error pkg-config required; use "apt-get install pkg-config" to install it.)
.PHONY: err
err: ; $(err)
endif
