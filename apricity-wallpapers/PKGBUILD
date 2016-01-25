#Maintainer: Alex Gajewski <agajews@gmail.com>

_pkgname='Apricity Wallpapers'
pkgname=apricity-wallpapers
pkgver=0.1.2
pkgrel=1
pkgdesc='Wallpapers for Apricity OS'
arch=(any)
license=(GPL)
url="https://github.com/agajews/ApricityOS"
depends=()
source=("apricity-wallpapers.tar.gz")
sha256sums=(SKIP)
install=apricity-wallpapers.install

package() {
	mkdir -p "${pkgdir}/usr/share/backgrounds"
	cp -rf "${srcdir}/apricity-wallpapers/backgrounds/apricity" "${pkgdir}/usr/share/backgrounds"
	mkdir -p "${pkgdir}/usr/share/gnome-background-properties"
	cp -f "${srcdir}/apricity-wallpapers/apricity-backgrounds.xml" "${pkgdir}/usr/share/gnome-background-properties"
}
