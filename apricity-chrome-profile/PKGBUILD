#Maintainer: Alex Gajewski <agajews@gmail.com>

_pkgname='Apricity Chrome Profile'
pkgname=apricity-chrome-profile
pkgver=0.1.2
pkgrel=1
pkgdesc='Default Google Chrome Profile for Apricity OS'
arch=(any)
license=(GPL)
url="https://github.com/agajews/ApricityOS"
depends=()
source=("apricity-chrome-profile.tar.gz")
sha256sums=(SKIP)

package() {
	mkdir -p "${pkgdir}/etc/apricity-assets"
	cp -rf "${srcdir}/apricity-chrome-profile/chrome-apps" "${pkgdir}/etc/apricity-assets"
	cp -rf "${srcdir}/apricity-chrome-profile/google-chrome" "${pkgdir}/etc/apricity-assets"
}
