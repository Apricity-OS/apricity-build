#Maintainer: Alex Gajewski <agajews@gmail.com>

_pkgname='Apricity Themes'
pkgname=apricity-themes-cinnamon
pkgver=0.0.6
pkgrel=1
pkgdesc='Cinnamon Assets for Apricity OS'
arch=(any)
license=(GPL)
url="https://github.com/agajews/ApricityOS"
depends=()
replaces=(apricity-themes)
conflicts=(apricity-themes, apricity-themes-gnome)
provides=(apricity-themes)
source=("apricity-themes-cinnamon.tar.gz")
sha256sums=(SKIP)
install="apricity-themes-cinnamon.install"

package() {
	rm -rf "${pkgdir}/usr/share/themes/Arctic Apricity"
	mkdir -p "${pkgdir}/usr/share/themes"
	mkdir -p "${pkgdir}/etc/skel/.config/autostart"
	cp -rf "${srcdir}/apricity-themes-cinnamon/Arctic Apricity" "${pkgdir}/usr/share/themes"
	cp -f "${srcdir}/apricity-themes-cinnamon/firstrun-desktop.sh" "${pkgdir}/etc/skel/.config/autostart"
}
