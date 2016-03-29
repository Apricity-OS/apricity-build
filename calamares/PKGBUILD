pkgname=calamares
_pointver=0
pkgver=2.0.${_pointver}
pkgrel=7
pkgdesc='Distribution-independent installer framework'
arch=('i686' 'x86_64')
license=(GPL)
url="https://github.com/calamares"
license=('LGPL')
depends=('kconfig' 'kcoreaddons' 'ki18n' 'solid' 'yaml-cpp' 'parted' 'boost' 'hwinfo' 'qt5-svg' 'polkit-qt5' 'gptfdisk' 'kdebase-runtime' 'rsync' 'python' 'dmidecode' 'ki18n' 'squashfs-tools' 'kpmcore-git')
makedepends=('extra-cmake-modules' 'qt5-tools' 'git')
backup=('usr/share/calamares/modules/bootloader.conf'
        'usr/share/calamares/modules/displaymanager.conf'
        'usr/share/calamares/modules/initcpio.conf'
        'usr/share/calamares/modules/unpackfs.conf')

source=("calamares.tar.gz")

sha256sums=(SKIP)

prepare() {
	cd "$srcdir/$pkgname"
	#sed -i -e "s|CALAMARES_VERSION_PATCH 0|CALAMARES_VERSION_PATCH $_pointver|g" CMakeLists.txt
}

build() {
	cd "$srcdir/$pkgname"

	mkdir -p build
	cd build
        cmake .. \
              -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_INSTALL_PREFIX=/usr \
              -DWITH_PARTITIONMANAGER=1 \
              -DCMAKE_INSTALL_LIBDIR=lib
        make
}

package() {
	cd "$srcdir/$pkgname/build"
	make DESTDIR="$pkgdir" install
	install -Dm644 "../settings.conf" "$pkgdir/usr/share/calamares/settings.conf"
	mkdir -p /usr/share/icons/
	install -Dm644 "../calamares.png" "$pkgdir/usr/share/icons/calamares.png"
	install -Dm644 "../calamares.desktop" "$pkgdir/usr/share/applications/calamares.desktop"
	rm -r "$pkgdir/usr/lib/calamares/modules/dummyprocess/"
	rm -r "$pkgdir/usr/lib/calamares/modules/dummypython/"
	rm "$pkgdir/usr/share/calamares/modules/dummypython.conf"
	cd "$srcdir/$pkgname"
	rm -r build
}
