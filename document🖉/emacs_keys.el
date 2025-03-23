;;  the sake of sanity...
;;
(global-set-key (kbd "C-z") nil) ;; turn off the poison C-z key.  Use C-x C-z or the command suspend-emacs
(global-set-key (kbd "C-v") nil) ;; tempting to put paste (yank) for a common typo, but at least lets not jump down the page
;; would be nice to clear C-c but minor modes redefine it


;;--------------------------------------------------------------------------------
;; extended character set for programming examples in the TTCA book
;;
;; preferable to use an Xcompose file definition when available
;;
(when t

  (global-set-key [f1] 'help-command)
  (global-set-key "\C-h" 'nil)
  (define-key key-translation-map (kbd "M-S") (kbd "§"))

  (global-set-key (kbd "C-x g copyright SPC") [?©]) 
    
  (global-set-key (kbd "C-x g phi SPC") [?φ]) ; phi for phase
  (global-set-key (kbd "C-x g Phi SPC") [?Φ]) 

  (global-set-key (kbd "C-x g d SPC") [?δ])
  (global-set-key (kbd "C-x g D SPC") [?Δ]) ; this is 'delta' is not 'increment'!
  (global-set-key (kbd "C-x g delta SPC") [?δ])
  (global-set-key (kbd "C-x g Delta SPC") [?Δ]) ; this is 'delta' is not 'increment'!

  (global-set-key (kbd "C-x g g SPC") [?γ])
  (global-set-key (kbd "C-x g G SPC") [?Γ])
  (global-set-key (kbd "C-x g gamma SPC") [?γ])
  (global-set-key (kbd "C-x g Gamma SPC") [?Γ])

  (global-set-key (kbd "C-x g l SPC") [?λ])
  (global-set-key (kbd "C-x g L SPC") [?Λ])
  (global-set-key (kbd "C-x g lambda SPC") [?λ])
  (global-set-key (kbd "C-x g Lambda SPC") [?Λ])

  (global-set-key (kbd "C-x g m SPC") [?μ])
  (global-set-key (kbd "C-x g M SPC") [?Μ])
  (global-set-key (kbd "C-x g mu SPC") [?μ])
  (global-set-key (kbd "C-x g Mu SPC") [?Μ])

  (global-set-key (kbd "C-x g p SPC") [?π])
  (global-set-key (kbd "C-x g P SPC") [?Π])
  (global-set-key (kbd "C-x g pi SPC") [?π])
  (global-set-key (kbd "C-x g Pi SPC") [?Π])

  (global-set-key (kbd "C-x g x SPC") [?ξ])
  (global-set-key (kbd "C-x g X SPC") [?Ξ])
  (global-set-key (kbd "C-x g xi SPC") [?ξ])
  (global-set-key (kbd "C-x g Xi SPC") [?Ξ])

  (global-set-key (kbd "C-x g > = SPC") [?≥])
  (global-set-key (kbd "C-x g < = SPC") [?≤])
  (global-set-key (kbd "C-x g ! = SPC") [?≠])
  (global-set-key (kbd "C-x g neq SPC") [?≠])
      
  (global-set-key (kbd "C-x g nil SPC") [?∅])

  (global-set-key (kbd "C-x g not SPC") [?¬])

  (global-set-key (kbd "C-x g and SPC") [?∧])
  (global-set-key (kbd "C-x g or SPC") [?∨])

  (global-set-key (kbd "C-x g exists SPC") [?∃])
  (global-set-key (kbd "C-x g all SPC") [?∀])

  (global-set-key (kbd "C-x g do SPC") [?⟳]) ; do
  (global-set-key (kbd "C-x g rb SPC") [?◨])
  (global-set-key (kbd "C-x g lb SPC") [?◧])

  (global-set-key (kbd "C-x g cont SPC") [?➜]) ; continue
  (global-set-key (kbd "C-x g thread SPC") [?☥]) ; thread

  (global-set-key (kbd "C-x g in SPC") [?∈]) ; set membership

  (global-set-key (kbd "C-x g times SPC") [?×]) ; set membership

  (global-set-key (kbd "C-x g cdot SPC") [?·]) ; scoping sepearator for gcc C

  (global-set-key (kbd "C-x g pencil SPC") [?🖉]) ; scoping sepearator for gcc C

)
