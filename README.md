# libShunkan [瞬間](https://ja.wikipedia.org/wiki/%E7%9E%AC%E9%96%93)

Lightweight game engine for Linux, aimed to have lowest possible latency and memory footprint.
Name comes from [japanese word](https://translate.google.com/translate?hl=en&sl=ja&tl=en&u=https%3A%2F%2Fja.wikipedia.org%2Fwiki%2F%E7%9E%AC%E9%96%93), meaning "moment", or "instant", because this work is dedicaded to achieving realtime performance.

## Getting Started, Prequisites 

I use Debian/Ubuntu for production system and prefer statical liknage. Wrappers are built on top of Linux kernel interfaces:
* For egl/gles graphics kms+drm+gbm backend is used.
* For event loop/networking and logic facilities like epoll, timerfd, evdev and sockets are used.

So following libraries would be required:
* libevdev

## Installing

Open terminal go like following:

```
sudo apt install build-essentials git 
cd ~/
git clone https://github.com/xakepp35/libshunkan.git
cd libshunkan 
make -j31337
echo I am ready ;-)
```

## Simpliest usage example

TBD
```
Example code would go here
```

## Contributing

Please read [CONTRIBUTING.md](https://github.com/xakepp35/libshunkan/blob/master/CONTRIBUTING.md) for details on the process for submitting pull requests.

## Authors

* **[xakepp35](https://github.com/xakepp35)** - *Initial work* - the gode of this code
* **[PurpleBooth](https://gist.github.com/PurpleBooth)** - Thank you for EXCELLENT.md templates ;-)

See also the list of [contributors](https://github.com/xakepp35/libshunkan/contributors) who participated in this project.

## License

This project is licensed under the FreeBSD 3-clause License.
See the [LICENSE](https://github.com/xakepp35/libshunkan/blob/master/LICENSE) file for details.
You may use [this email address](mailto:xakepp35@gmail.com) to contact me if any questions.

## Acknowledgments

gl;hf
