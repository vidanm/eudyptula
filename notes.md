# Linux Devices Drivers 3 Notes
## Chapitre 2 : 1er programme et compilation
- vmalloc
- Makefile syntaxe particulière avec
    obj-m := module.o
    module-objs := file1.o file2.o
et vérifications de si c'est l'utilisateur qui compile ou le noyau avec :
    ifneq ($(KERNELRELEASE),)
- chargement des modules avec insmod
- déchargement d'un module avec rmmod (S'il n'est pas en cours d'utilisation)
- modprobe permets aussi de charger un module mais va charger également tous les modules nécessaires (si présents dans le répertoire des modules standards) au module que l'on veut charger
- dmesg : logs du kernel
- les printk s'affichent dans dmesg 
- lsmod liste les modules chargés dans le kernel
- Différentes macros disponibles pour gérer les versions d'un module (cf. p26)
- Pour l'exports de symboles :
    `EXPORT_SYMBOL(name);`
    `EXPORT_SYMBOL_GPL(name);`
- le `_GPL` permets de rendre accessible uniquement au module sous licence GPL
- Les lignes obligatoires dans un module.c :
```
#include <linux/module.h>
#include <linux/init.h>
MODULE_LICENSE("GPL"); //Pas obligatoire mais mieux + propriétaire par défaut
```
- Gestion d'erreur primordiale avec désenregistrement des ressources si fail (goto autorisé) :
```
else
    goto fail;

fail:
    my_cleanup();
```
- "Your code must be prepared to be called as soon as it completes its first registration"
- Possibilité d'ajouter des parametres de modules avec `module_param(name,type,permission)` (cf. p36-37)

## Chapitre 3 : Sculls
- Un scull c'est un char device driver qui n' est pas dependant de la machine
- Simple Character Utility for Loading Localities
- Differents types de scull explicites p43
- Les devices sont des fichiers speciaux : ls -l nous donne le type dans la premiere colonne c char driver b block device...
- Le numero de ces devices separes par une virgule explicite pour le premier le driver qui les manage, pour le deuxiemem (minor) est un indice ou un pointeur vers le device du noyau
- The kernel uses the major number at open time to dispatch execution to the appropriate driver. The minor number is used only by the driver specified by the major number
- les numeros majeur et mineurs peuvent s'obtenir en C grace aux fonctions `MAJOR(dev_t de1v) MINOR(..)`
- Operation inverse avec `MKDEV(int major, int minor);`
- Enregistrement de device char avec `register_chrdev_region(dev_t first, unsigned int count, char*name);`
- FILE comme on l'entends dans la lib C est différent des file structures du kernel
- filp => file pointer

### Char Device Registration 
- Une inode c'est une structure qui contient des informations à propos de nos file
- elle contient entre autre deux parametres important :
- `i_rdev` contient le device number
- `i_cdev' contient un pointer vers la structure qui représente le char device
- La structure qui représente les char devices est `struct cdev`
- Pour que le noyau puisse utiliser les operations de notre device il faut allouer et enregistrer une ou plueisuere de ces structures
- cdev_alloc() => cdev_init(...) => cdev_add pour notifier son existence au kernel (l'enregistrer)

## Chapitre 14 : Linux Device Model
### Kobjects
- Structure fondamentale permettant de synthetiser le device model
- Permets de savoir quand un objet de kernel peut etre supprimes (plus de references vers l'objet)
- Notifie l'user space des aller et venus de l'hardware du systeme
- Recuperer le pointer cdev d'un kobject : `struct cdev \*device = container\_of(kp,struct cdev, kobj);`
- Il peut etre interessant de creer des macros pour back-caster les kobjects vers la structure conteneur

#### Initialisation d'un kobject
- 1ère étape initialiser l'objet à 0 avec memset
- 2ème étape utiliser `kobject\_init(struct kobject \*kobj)`
- 3ème étape au minimum renseigner le nom du kobject qui sera utilisé dans les entrées sysfs : `kobject\_set\_name(struct kobject \*kobj, const char \*format,...); //Utilisation comme printk`
- struct kobject *kobject_get(struct kobject *kobj) incremente le compteur de reference de kobj et return un pointeur vers le kobject
- kobject_put decremente le compteur et possiblement, free l'objet
- on ne peut pas assurer à quel moment le compteur arrivera à 0 par conséquent on se sert d'une méthode release qui sera apellé de manière asynchrone et qui liberera les ressources (cf. p367)
- cette méthode release est à définir dans une struct kobj_type, chaque kobject doit être associé à un kobj_type

#### Parent pointer et Ksets
- Dans un kobject il y'a un champ parent qui référence le parent direct de l'objet (ex USB dev -> USB Hub dev )
- Un kset est une sorte de kobject qui en référence d'autres comme une collection de kobjects
- Ajouter un kobject à un kset se fait via la fonction `kobject\_add(struct kobject \*kobj)`
- Il existe aussi la fonction `kobject\_register(struct kobject \*kobj)` qui regroupe les fonctions kobject\_init et kobject\_add
- kobject_del pour le suppr du kset (ou kobject_unregister)
- schema disponible p370

#### Attributs par défaut
- Les Kobject ont des attributs par défaut défini dans leur kobj_type associés
- Ces attribus par défaut doivent être définis et seront crées pour chaque kobjet de ce type
- kobj_type -> sysfs_ops permets d'implémenter les méthodes show et store pour les attributs par défaut
- Pour show il faut que la valeur soit encodé dans `buffer` et qu'elle retourne la taille des données de buffer (max PAGE_SIZE)
- La méthode show est la même pour tous les attribus d'un kobject donné
- La méthode store decode les données de buffer puis la stocke et renvoi le nombre d'octets décodés
- Il vaut mieux renvoyer un nombre négatif s'il y'a une erreur dans les données en argument que risquer des problèmes plus difficiles

#### Autres attributs
- En général un kobject n'a pas besoin d'attributs supplémentaires à ceux par défaut mais les attributs peuvent être ajoutés et supprimés à guise (Pour ajouter un attribut voir p374)

#### Attributs binaires
- `struct bin\_attribute`
- Nécessaires quand les données sont conséquentes et ne doivent pas être modifiés
- Ils ne peuvent pas être compris dans les attributs par défaut et doivent être crées à part (p375)

#### Hotplugs
- Invocation de `/sbin/hotplug/`
- `struct kset\_hotplug\_ops` (p376)
- la méthode filter permets de filtrer quels événements concernant l'hotplug doit être passé côté user space
- si filter return 0 l'event n'est pas créé
- La méthode hotplug permets d'ajouter des variables d'environnements 
- Si on ajoute des variables à envp il faut impérativement ajouter NULL à la fin pour prévenir le kernel de la fin de nos ajouts
- La valeur de retour de hotplug doit être 0

#### Buses, Devices, Drivers
- bus : `struct bus\_type`
- voir lddbus
- 

#### Divers
- Possibilité de créer des liens symboliques
