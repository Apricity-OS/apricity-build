#Maintainer: Alex Gajewski <agajews@gmail.com>

_pkgname='Apricity Themes'
pkgname=apricity-themes-gnome
pkgver=0.1.9
pkgrel=1
pkgdesc='Gnome Assets for Apricity OS'
arch=(any)
license=(GPL)
url="https://github.com/agajews/ApricityOS"
depends=()
replaces=(apricity-themes)
conflicts=(apricity-themes, apricity-themes-cinnamon)
provides=(apricity-themes)
source=("apricity-themes-gnome.tar.gz")
sha256sums=(SKIP)
install="apricity-themes-gnome.install"

package() {
	rm -rf "${pkgdir}/usr/share/themes/Arctic Apricity"
	mkdir -p "${pkgdir}/usr/share/themes"
	mkdir -p "${pkgdir}/etc/skel/.config/autostart"
	cp -rf "${srcdir}/apricity-themes-gnome/Arctic Apricity" "${pkgdir}/usr/share/themes"
	cp -f "${srcdir}/apricity-themes-gnome/firstrun-desktop.sh" "${pkgdir}/etc/skel/.config/autostart"
}
