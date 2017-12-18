# git format-patch -M --subject-prefix='PATCH net' -1 \

git format-patch -M --subject-prefix='RFC' -1 \
  `#-v 2` \
  `#--cover-letter` \
  $1

