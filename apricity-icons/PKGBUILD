#Maintainer: Alex Gajewski <agajews@gmail.com>

_pkgname='Apricity Icons'
pkgname=apricity-icons
pkgver=0.1.4
pkgrel=1
pkgdesc='Icons for Apricity OS'
arch=(any)
license=(GPL)
url="https://github.com/agajews/ApricityOS"
depends=()
source=("apricity-icons.tar.gz")
sha256sums=(SKIP)

package() {
	mkdir -p "${pkgdir}/usr/share/icons"
	cp -rf "${srcdir}/apricity-icons/Apricity Icons" "${pkgdir}/usr/share/icons"
}
