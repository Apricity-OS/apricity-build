#Maintainer: Alex Gajewski <apagajewski@gmail.com>

_pkgname=ice-ssb
pkgname=ice-ssb
pkgver=5.0.9
pkgrel=1
pkgdesc='ICE SSB'
arch=(any)
license=(GPL)
depends=("python-requests" "python-beautifulsoup4" "python2" "pygtk" "python2-gobject")
source=("ice-ssb.tar.gz")
sha256sums=(SKIP)
install=ice-ssb.install

package() {
	mkdir -p "${pkgdir}/usr/share/applications"
	mkdir -p "${pkgdir}/usr/share/pixmaps"
	mkdir -p "${pkgdir}/usr/bin"
	cp -f "${srcdir}/ice-ssb/ice" "${pkgdir}/usr/bin"
	cp -f "${srcdir}/ice-ssb/ice-firefox" "${pkgdir}/usr/bin"
	cp -f "${srcdir}/ice-ssb/ice.desktop" "${pkgdir}/usr/share/applications"
	cp -f "${srcdir}/ice-ssb/ice.png" "${pkgdir}/usr/share/pixmaps"
}
