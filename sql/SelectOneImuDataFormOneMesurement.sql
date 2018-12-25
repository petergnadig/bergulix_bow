select 
time_raw,
acc_x,
acc_y,
acc_z,
imu_serial,
mdate
from m_data 
left join m_imu on fk_id_m_imu=id_m_imu
left join m_head on fk_id_m_head=id_m_head
where 
  mdate='2018-11-29 21:27:05' and 
  imu_serial='360aad'
order by time_raw, imu_serial