
patch=$1

# echo "patch: \"$patch\""

if [ -z $patch ] || [ ! -f $patch ] ; then
  echo "WTF?  Where's my file, bitch?"
  exit
fi

# git send-email --cc-cmd='./scripts/get_maintainer.pl --interactive --no-git-fallback' --no-chain-reply-to \
git send-email --no-chain-reply-to --suppress-cc=all \
  --to='octeontx2-dev <octeontx2-dev@caviumnetworks.microsoftonline.com>' \
  --cc="ncg-sw-sunilg@caviumnetworks.microsoftonline.com" \
  --cc='"Goutham, Sunil" <Sunil.Goutham@cavium.com>' \
  --cc="Aleksey Makarov <aleksey.makarov@auriga.com>" \
  --cc="Aleksey Makarov <aleksey.makarov@cavium.com>" \
  $patch

exit

  --dry-run \
  --in-reply-to='20180207130639.9844-1-aleksey.makarov@cavium.com' \

  --to='octeontx2-dev <octeontx2-dev@cavium.com>' \
  --to="netdev@vger.kernel.org" \

  --cc="linux-kernel@vger.kernel.org" \
  --cc="Robert Richter <rric@kernel.org>" \
  --cc="Aleksey Makarov <aleksey.makarov@cavium.com>" \
  --cc="cjacob <cjacob@caviumnetworks.com>" \
  --cc="ncg-sw-sunilg@caviumnetworks.microsoftonline.com" \
  --cc="Florian Westphal <fw@strlen.de>" \
  --cc="linux-arm-kernel@lists.infradead.org" \
  --cc="Colin Ian King <colin.king@canonical.com>" \
